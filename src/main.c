#include <curl/curl.h>
#include <libxml2/libxml/HTMLparser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FF_USERAGENT "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:99.0) Gecko/20100101 Firefox/99.0"
#define BASEURL "https://flareboard.ru/"
#define MAX_LINKS 30

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
        fprintf(stderr, "curl_easy_perform() failed: %s", curl_easy_strerror(response));
        return 0;
    }

    char** links = malloc(MAX_LINKS * sizeof(char*));
    for (int i = 0; i < MAX_LINKS; i++) links[i] = NULL;

    xmlDocPtr doc = htmlReadDoc((xmlChar*)memory.data, NULL, NULL,
                                HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    xmlNodePtr cur = xmlDocGetRootElement(doc)->children->children;  // <html> <-- <head> <-- <node>
    int j = 0;
    while (cur) {
        if (xmlStrcmp(cur->name, (xmlChar*)"link") == 0) {
            xmlChar* attr = xmlGetProp(cur, (xmlChar*)"href");

            if (attr) {
                size_t base_len = strlen(BASEURL);
                size_t attr_len = strlen((char*)attr);

                links[j] = malloc((base_len + attr_len + 1) * sizeof(char));
                strcpy(links[j], BASEURL);
                strcat(links[j], (char*)attr);
                links[j][base_len + attr_len] = '\0';

                xmlFree(attr);
                j++;
            }
        }

        cur = cur->next;
    }

    for (int i = 0; links[i]; i++) {
        printf("%s\n", links[i]);
    }

    free(memory.data);
    for (int i = 0; i < MAX_LINKS; i++) free(links[i]);
    free(links);
    xmlFreeDoc(doc);
    curl_global_cleanup();
    return 0;
}
