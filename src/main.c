#include <curl/curl.h>
#include <libxml2/libxml/HTMLparser.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "linkpool.h"

#define FF_USERAGENT "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:99.0) Gecko/20100101 Firefox/99.0"
#define BASEURL "https://flareboard.ru/"

struct MemoryStruct {
    char* data;
    size_t size;
};

size_t write_memory_callback(void* contents, size_t size, size_t nmemb, struct MemoryStruct* userdata) {
    if (!userdata) {
        fprintf(stderr, "%s: userdata is NULL\n", __FUNCTION__);
        return 0;
    }

    size_t realsize = size * nmemb;
    char* ptr = realloc(userdata->data, userdata->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "%s: not enough memory\n", __FUNCTION__);
        return 0;
    }

    userdata->data = ptr;
    memcpy(&(userdata->data[userdata->size]), contents, realsize);
    userdata->size += realsize;
    userdata->data[userdata->size] = 0;

    return realsize;
}

size_t write_file_callback(char* ptr, size_t size, size_t nmemb, FILE* fd) {
    if (!fd) {
        fprintf(stderr, "%s: file descriptor is NULL\n", __FUNCTION__);
        return 0;
    }

    return fwrite(ptr, size, nmemb, fd);
}

int save_url_contents(char* url, char* filename) {
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
    }

    curl_global_cleanup();
    return 0;
}

int str_find(char* str, char symbol) {
    if (!str) return -1;

    int i = 0;
    while (str[i]) {
        if (str[i] == symbol) {
            return i;
        }

        i++;
    }

    return -1;
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl_handle = curl_easy_init();

    //    const char* baseurl = "https://flareboard.ru/thread.php?id=1327";
    const char* url = "https://flareboard.ru/thread.php?id=2666";
    struct MemoryStruct memory = {malloc(1), 0};

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 0);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&memory);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, FF_USERAGENT);

    CURLcode response = curl_easy_perform(curl_handle);
    curl_easy_cleanup(curl_handle);

    if (response != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() for %s failed: %s\n", url, curl_easy_strerror(response));
        return 0;
    }

    LinkPool* links = linkpool_create();
    xmlDocPtr doc = htmlReadDoc((xmlChar*)memory.data, NULL, NULL,
                                HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);

    // <html> <-- <head> <-- <node> --- <node>
    xmlNodePtr cur = xmlDocGetRootElement(doc)->children->children;
    while (cur) {
        if (xmlStrcmp(cur->name, (xmlChar*)"link") == 0) {
            xmlChar* attr = xmlGetProp(cur, (xmlChar*)"href");

            if (attr && !(strstr((char*)attr, "https://") == (char*)attr ||
                          strstr((char*)attr, "http://") == (char*)attr)) {
                int slash_pos = str_find((char*)attr, '/');
                if (slash_pos >= 0) {
                    attr[slash_pos] = '\0';
                    mkdir((char*)attr, 0755);
                    attr[slash_pos] = '/';
                }

                size_t bufsize = strlen(BASEURL) + strlen((char*)attr) + 1;
                char buf[bufsize];
                snprintf(buf, bufsize, "%s%s%c", BASEURL, attr, '\0');

                links = linkpool_push_node(links, buf, (char*)attr);
                xmlFree(attr);
            }
        }

        cur = cur->next;
    }

    linkpool_print(links);

    // TODO: Save files
    // xmlSaveFormatFileEnc("output.html", doc, "UTF-8", 1);

    free(memory.data);
    linkpool_free(links);
    xmlFreeDoc(doc);
    curl_global_cleanup();
    return 0;
}
