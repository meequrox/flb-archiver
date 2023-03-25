#include <curl/curl.h>
#include <libxml2/libxml/HTMLparser.h>
#include <libxml2/libxml/xpath.h>
#include <libxml2/libxml/xpathInternals.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "linkpool.h"
#include "memory.h"

#define FF_USERAGENT "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:99.0) Gecko/20100101 Firefox/99.0"
#define BASEURL "https://flareboard.ru/"

int save_url_contents(char* url, char* filename, int verbose) {
    if (!url || !filename) return 1;

    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl_handle = curl_easy_init();

    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "%s: can't open %s\n", __FUNCTION__, filename);
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
    } else if (verbose)
        fprintf(stdout, "%s -> %s\n", url, filename);

    fclose(file);
    curl_global_cleanup();
    return 0;
}

int download_links(LinkPool* pool, int verbose) {
    if (!pool) return 1;

    LinkNode* cur = pool->head;

    while (cur) {
        save_url_contents(cur->url, cur->filename, verbose);
        cur = cur->next;
    }

    return 0;
}

int str_find_nth(char* str, char symbol, int n) {
    if (!str) return -1;

    int count = 0;
    int i = 0;
    while (str[i]) {
        if (str[i] == symbol) count++;
        if (count == n) return i;
        i++;
    }
    return -1;
}

int create_subdirs(const char* path) {
    if (!path) return 1;
    size_t bufsize = strlen(path) + 1;
    char buf[bufsize];
    char* buf_ptr = buf;
    strncpy(buf_ptr, path, bufsize);

    int n = 1;
    int slash_pos = str_find_nth(buf_ptr, '/', n);
    while (slash_pos >= 0) {
        buf_ptr[slash_pos] = '\0';
        mkdir(buf_ptr, 0755);
        buf_ptr[slash_pos] = '/';

        n++;
        slash_pos = str_find_nth(buf_ptr, '/', n);
    }

    return 0;
}

LinkPool* links = NULL;

int process_attr(char* attr) {
    create_subdirs((char*)attr);

    size_t url_bufsize = strlen(BASEURL) + strlen((char*)attr) + 1;
    char url_buf[url_bufsize];
    snprintf(url_buf, url_bufsize, "%s%s%c", BASEURL, attr, '\0');
    links = linkpool_push_node(links, url_buf, (char*)attr);

    xmlFree(attr);
}

int save_thread(unsigned int id) {
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl_handle = curl_easy_init();

    size_t bufsize = strlen(BASEURL) + strlen("thread.php?id=") + 17;
    char buf[bufsize];
    snprintf(buf, bufsize, BASEURL "thread.php?id=%d", id);
    struct MemoryStruct memory = {malloc(1), 0};

    curl_easy_setopt(curl_handle, CURLOPT_URL, buf);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 0);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&memory);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, FF_USERAGENT);

    CURLcode response = curl_easy_perform(curl_handle);
    curl_easy_cleanup(curl_handle);

    if (response != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() for %s failed: %s\n", buf, curl_easy_strerror(response));
        return 1;
    }

    xmlDocPtr doc = htmlReadDoc((xmlChar*)memory.data, NULL, NULL,
                                HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);

    // <html> <-- <head> <-- <node> --- <node>
    xmlNodePtr cur = xmlDocGetRootElement(doc)->children->children;
    while (cur) {
        int is_link = xmlStrcmp(cur->name, (xmlChar*)"link") == 0;
        int is_script = xmlStrcmp(cur->name, (xmlChar*)"script") == 0;

        if (is_link || is_script) {
            xmlChar* attr = NULL;
            if (is_link)
                attr = xmlGetProp(cur, (xmlChar*)"href");
            else
                attr = xmlGetProp(cur, (xmlChar*)"src");

            if (attr && !(strstr((char*)attr, "https://") == (char*)attr ||
                          strstr((char*)attr, "http://") == (char*)attr)) {
                process_attr((char*)attr);
            }
        }

        cur = cur->next;
    }

    xmlXPathContextPtr context = xmlXPathNewContext(doc);
    xmlXPathRegisterNs(context, (xmlChar*)"html", (xmlChar*)"http://www.w3.org/1999/xhtml");

    xmlXPathObjectPtr result_img = xmlXPathEvalExpression((xmlChar*)"//img", context);
    xmlNodeSetPtr nodes_img = result_img->nodesetval;
    for (int i = 0; i < nodes_img->nodeNr; i++) {
        xmlNodePtr node_img = nodes_img->nodeTab[i];
        xmlChar* src = xmlGetProp(node_img, (xmlChar*)"src");
        if (src) process_attr((char*)src);
    }

    xmlXPathObjectPtr result_video = xmlXPathEvalExpression((xmlChar*)"//video", context);
    xmlNodeSetPtr nodes_video = result_video->nodesetval;
    for (int i = 0; i < nodes_video->nodeNr; i++) {
        xmlNodePtr node_video = nodes_video->nodeTab[i];
        xmlChar* src = xmlGetProp(node_video, (xmlChar*)"src");
        if (src) process_attr((char*)src);
    }

    snprintf(buf, bufsize, "%d.html", id);
    FILE* file = fopen(buf, "w");
    fprintf(file, "%s\n", memory.data);
    fclose(file);

    free(memory.data);
    xmlXPathFreeObject(result_img);
    xmlXPathFreeObject(result_video);
    xmlXPathFreeContext(context);
    xmlFreeDoc(doc);
    curl_global_cleanup();
    return 0;
}

void mk_flbdir(void) {
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);

    char dirname_buf[32];
    snprintf(dirname_buf, 32, "flb_%02d.%02d.%02d_%d", tm->tm_mday, tm->tm_mon + 1,
             (tm->tm_year + 1900) % 2000, tm->tm_hour * tm->tm_min + tm->tm_sec);

    mkdir(dirname_buf, 0755);
    chdir(dirname_buf);

    fprintf(stdout, "Working directory: %s\n", dirname_buf);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stdout, "Usage: %s START_ID END_ID\n", argv[0]);
        fprintf(stdout, "Example: %s 151 1337\n", argv[0]);
        return 0;
    }

    links = linkpool_create();
    mk_flbdir();

    unsigned int lb = strtoul(argv[1], NULL, 10);
    unsigned int ub = strtoul(argv[2], NULL, 10);

    fprintf(stdout, "Download pages from %d to %d\n", lb, ub);
    for (int id = lb; id <= ub; id++) {
        save_thread(id);

        // Sleep 300ms
        usleep(300.0 * 1000);
    }
    download_links(links, 1);

    linkpool_free(links);
    return 0;
}
