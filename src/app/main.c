#include <curl/curl.h>
#include <libxml2/libxml/HTMLparser.h>
#include <libxml2/libxml/xmlsave.h>
#include <libxml2/libxml/xpath.h>
#include <libxml2/libxml/xpathInternals.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "linkpool/linkpool.h"
#include "memory/memory.h"

#define FF_USERAGENT "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:99.0) Gecko/20100101 Firefox/99.0"
#define BASEURL "https://flareboard.ru/"

LinkPool* links = NULL;

void cd_flbdir(void);
int str_find_nth(const char* str, char symbol, int n);
int save_url_contents(char* url, char* filename, int verbose);
int download_links(LinkPool* pool, int verbose);
int create_subdirs(const char* path);

int process_attrs(xmlXPathContext* context, const char* xpath_expr, const char* needed_attr) {
    if (!xpath_expr || !needed_attr) {
        return 1;
    }

    xmlXPathObject* result = xmlXPathEvalExpression((xmlChar*) xpath_expr, context);
    xmlNodeSet* nodes = result->nodesetval;

    for (int i = 0; i < nodes->nodeNr; ++i) {
        xmlNode* node = nodes->nodeTab[i];

        xmlChar* attr = xmlGetProp(node, (xmlChar*) needed_attr);

        if (attr && !((char*) attr == strstr((char*) attr, "https://") ||
                      (char*) attr == strstr((char*) attr, "http://"))) {
            create_subdirs((char*) attr);

            size_t url_bufsize = strlen(BASEURL) + strlen((char*) attr) + 1;

            char url_buf[url_bufsize];
            snprintf(url_buf, url_bufsize, "%s%s%c", BASEURL, attr, '\0');

            links = linkpool_push_node(links, url_buf, (char*) attr);
            xmlFree(attr);
        }
    }

    xmlXPathFreeObject(result);
    return 0;
}

int page_is_thread(xmlXPathContext* context) {
    xmlXPathObject* result = xmlXPathEvalExpression((xmlChar*) "//div[@class='postTop']", context);
    xmlNodeSet* nodes = result->nodesetval;
    int nnodes = nodes->nodeNr;

    xmlXPathFreeObject(result);
    return nnodes > 0;
}

int save_thread(unsigned int id) {
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl_handle = curl_easy_init();
    if (!curl_handle) {
        fprintf(stderr, "%s: can not handle curl\n", __func__);
        curl_global_cleanup();
        return 1;
    }

    size_t bufsize = strlen(BASEURL) + strlen("thread.php?id=") + 17;

    char buf[bufsize];
    snprintf(buf, bufsize, BASEURL "thread.php?id=%d", id);

    struct MemoryStruct memory = {malloc(1), 0};

    curl_easy_setopt(curl_handle, CURLOPT_URL, buf);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 0);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*) &memory);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, FF_USERAGENT);

    CURLcode response = curl_easy_perform(curl_handle);
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    if (response != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() for %s failed: %s\n", buf, curl_easy_strerror(response));
        return 1;
    }

    xmlDoc* doc = htmlReadDoc((xmlChar*) memory.data, NULL, NULL,
                              HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    xmlXPathContext* context = xmlXPathNewContext(doc);
    xmlXPathRegisterNs(context, (xmlChar*) "html", (xmlChar*) "http://www.w3.org/1999/xhtml");

    if (page_is_thread(context)) {
        process_attrs(context, "//link", "href");
        process_attrs(context, "//script", "src");
        process_attrs(context, "//img", "src");
        process_attrs(context, "//video", "src");

        snprintf(buf, bufsize, "%d.html", id);

        xmlSaveCtxt* saveCtxt = xmlSaveToFilename(
            buf, "UTF-8", XML_SAVE_FORMAT | XML_SAVE_NO_DECL | XML_SAVE_NO_EMPTY | XML_SAVE_AS_HTML);
        xmlSaveDoc(saveCtxt, doc);
        xmlSaveClose(saveCtxt);
    } else {
        fprintf(stderr, "%s: thread with id %d does not exist\n", __func__, id);
    }

    free(memory.data);
    xmlFreeDoc(doc);
    xmlXPathFreeContext(context);
    return 0;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stdout, "Usage: %s START_ID END_ID\n", argv[0]);
        fprintf(stdout, "Example: %s 151 1337\n", argv[0]);
        return 0;
    }

    links = linkpool_create();
    cd_flbdir();

    unsigned int lb = strtoul(argv[1], NULL, 10);
    unsigned int ub = strtoul(argv[2], NULL, 10);

    fprintf(stdout, "Download pages from %d to %d\n", lb, ub);
    for (unsigned int id = lb; id <= ub; ++id) {
        save_thread(id);

        const int usecs = 300.0 * 1000;
        usleep(usecs);
    }

    fprintf(stdout, "All pages saved! Start downloading extra links (img, video)\n");
    download_links(links, 0);

    linkpool_free(links);
    return 0;
}

void cd_flbdir(void) {
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);

    int short_year = (tm->tm_year + 1900) % 2000;

    char dirname_buf[32];
    snprintf(dirname_buf, 32, "flb_%02d.%02d.%02d_%02d-%02d", short_year, tm->tm_mon + 1, tm->tm_mday,
             tm->tm_hour, tm->tm_min);

    mkdir(dirname_buf, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    chdir(dirname_buf);

    fprintf(stdout, "Working directory: %s\n", dirname_buf);
}

int str_find_nth(const char* str, char symbol, int n) {
    if (!str) {
        return -1;
    }

    int count = 0;
    int i = 0;
    while (str[i]) {
        if (str[i] == symbol) {
            ++count;
        }
        if (count == n) {
            return i;
        }
        ++i;
    }
    return -1;
}

int save_url_contents(char* url, char* filename, int verbose) {
    if (!url || !filename) {
        return 1;
    }

    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl_handle = curl_easy_init();
    if (!curl_handle) {
        fprintf(stderr, "%s: can not handle curl\n", __func__);
        curl_global_cleanup();
        return 1;
    }

    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "%s: can't open %s\n", __func__, filename);
        return 1;
    }

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_file_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, FF_USERAGENT);

    CURLcode response = curl_easy_perform(curl_handle);
    curl_easy_cleanup(curl_handle);

    if (response != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() for %s failed: %s\n", url, curl_easy_strerror(response));
        return 1;
    }

    if (verbose) {
        fprintf(stdout, "%s -> %s\n", url, filename);
    }

    fclose(file);
    curl_global_cleanup();
    return 0;
}

int download_links(LinkPool* pool, int verbose) {
    if (!pool) {
        return 1;
    }

    LinkNode* cur = pool->head;

    while (cur) {
        save_url_contents(cur->url, cur->filename, verbose);
        cur = cur->next;
    }

    return 0;
}

int create_subdirs(const char* path) {
    if (!path) {
        return 1;
    }

    size_t bufsize = strlen(path) + 1;
    char buf[bufsize];
    char* buf_ptr = buf;
    strncpy(buf_ptr, path, bufsize);

    int n = 1;
    int slash_pos = str_find_nth(buf_ptr, '/', n);
    while (slash_pos >= 0) {
        buf_ptr[slash_pos] = '\0';
        mkdir(buf_ptr, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

#if defined(_WIN32)
        buf_ptr[slash_pos] = '\\';
#else
        buf_ptr[slash_pos] = '/';
#endif

        slash_pos = str_find_nth(buf_ptr, '/', ++n);
    }

    return 0;
}
