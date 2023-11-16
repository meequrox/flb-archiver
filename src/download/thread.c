#include "download/thread.h"

#include <errno.h>
#include <libxml/HTMLparser.h>
#include <libxml/xmlsave.h>
#include <libxml2/libxml/xpathInternals.h>
#include <string.h>
#include <unistd.h>

#include "data_structures/rbtree.h"
#include "download/comments.h"
#include "filesystem/directories.h"
#include "logger/logger.h"
#include "memory/memory.h"

const char kBaseUrl[] = "https://flareboard.ru/";
const size_t kBaseUrlLen = sizeof(kBaseUrl) / sizeof(*kBaseUrl);

const char kFirefoxUserAgent[] =
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:99.0) Gecko/20100101 Firefox/99.0";

int flb_is_thread(xmlXPathContext* context) {
    const xmlChar* expr = (xmlChar*) "//div[@class='postTop']";

    xmlXPathObject* result = xmlXPathEvalExpression(expr, context);
    const int nodes = result->nodesetval->nodeNr;

    xmlXPathFreeObject(result);
    return nodes > 0;
}

static int download_resource(CURL* curl_handle, const char* url, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        FLB_LOG_ERROR("'%s': %s", filename, strerror(errno));
        return 1;
    }

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, flb_write_file_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, file);

    CURLcode response = curl_easy_perform(curl_handle);

    if (response != CURLE_OK) {
        FLB_LOG_ERROR("Can't fetch '%s': %s", url, curl_easy_strerror(response));
        return 1;
    }

    FLB_LOG_INFO("Downloaded '%s' to file '%s'", url, filename);

    fclose(file);
    return 0;
}

static int download_resources(CURL* curl_handle, flb_rbtree* tree, flb_rbnode* root) {
    if (!curl_handle || !root || root == tree->leaf) {
        return 0;
    }

    const char* url = root->key;
    const char* filename = root->value;

    download_resource(curl_handle, url, filename);

    download_resources(curl_handle, tree, root->left);
    download_resources(curl_handle, tree, root->right);

    return 0;
}

static int is_local_path(const char* path) {
    return path != strstr(path, "https://") && path != strstr(path, "http://");
}

static int parse_resources(flb_rbtree* tree, xmlXPathContext* context, const char* xpath_expr,
                           const xmlChar* find_attr) {
    if (!tree || !xpath_expr || !find_attr) {
        return 1;
    }

    xmlXPathObject* result = xmlXPathEvalExpression((xmlChar*) xpath_expr, context);
    xmlNodeSet* nodes = result->nodesetval;

    for (int i = 0; i < nodes->nodeNr; ++i) {
        xmlChar* attr = xmlGetProp(nodes->nodeTab[i], find_attr);
        char* attr_str = (char*) attr;

        if (attr && is_local_path(attr_str)) {
            if (access(attr_str, F_OK) != 0) {
                flb_mkdirs(attr_str);

                const size_t resource_url_len = kBaseUrlLen + strlen(attr_str) + 1;

                char resource_url[resource_url_len];
                snprintf(resource_url, resource_url_len, "%s%s", kBaseUrl, attr);

                flb_rbtree_insert(tree, resource_url, attr_str);
                FLB_LOG_INFO("Insert '%s' into resources tree (file '%s')", resource_url, attr_str);
            }

            xmlFree(attr);
            attr = NULL;
        }
    }

    xmlXPathFreeObject(result);
    return 0;
}

static void download_thread_cleanup(char* data, xmlDoc* doc, xmlXPathContext* context,
                                    flb_rbtree* tree) {
    free(data);
    xmlFreeDoc(doc);
    xmlXPathFreeContext(context);
    flb_rbtree_free(tree);
}

int flb_download_thread(CURL* curl_handle, size_t id) {
    const char query_base[] = "thread.php?id=";
    const size_t query_base_len = sizeof(query_base) / sizeof(*query_base);

    const size_t uint_max_digits = 10;
    const size_t bufsize = kBaseUrlLen + query_base_len + uint_max_digits + 1;

    char thread_url[bufsize];
    snprintf(thread_url, bufsize, "%s%s%zu", kBaseUrl, query_base, id);

    const size_t initial_bufsize = 32 * 1024 + 1;
    flb_memstruct_t memory = {(char*) malloc(initial_bufsize), 0, initial_bufsize};

    curl_easy_setopt(curl_handle, CURLOPT_URL, thread_url);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 0);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, flb_write_memory_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*) &memory);

    CURLcode response = curl_easy_perform(curl_handle);

    if (response != CURLE_OK) {
        FLB_LOG_ERROR("Can't fetch %s: %s", thread_url, curl_easy_strerror(response));
        download_thread_cleanup(memory.data, NULL, NULL, NULL);
        return 1;
    }

    const int parse_options = HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING;
    xmlDoc* doc = htmlReadDoc((xmlChar*) memory.data, NULL, NULL, parse_options);

    xmlXPathContext* context = xmlXPathNewContext(doc);
    xmlXPathRegisterNs(context, (xmlChar*) "html", (xmlChar*) "http://www.w3.org/1999/xhtml");

    flb_rbtree* resources_tree = NULL;

    if (flb_is_thread(context)) {
        resources_tree = flb_rbtree_create();

        if (!resources_tree) {
            FLB_LOG_ERROR("Can't create resources tree");
            download_thread_cleanup(memory.data, doc, context, NULL);
            return 1;
        }

        FLB_LOG_INFO_LB("Processing thread %zu (url '%s')", id, thread_url);

        // TODO(1): fetch thread comments
        flb_dummy();

        parse_resources(resources_tree, context, "//link", (xmlChar*) "href");
        parse_resources(resources_tree, context, "//script", (xmlChar*) "src");
        parse_resources(resources_tree, context, "//img", (xmlChar*) "src");
        parse_resources(resources_tree, context, "//video", (xmlChar*) "src");

        if (resources_tree && resources_tree->root) {
            FLB_LOG_INFO("Thread %d contains downloadable resources", id);

            // TODO(3): Implement universal DFS flb_rbtree_foreach(func, tree, root) function
            download_resources(curl_handle, resources_tree, resources_tree->root);
        }

        const char extension[] = ".html";
        char filename[uint_max_digits + sizeof(extension) / sizeof(*extension) + 1];
        snprintf(filename, bufsize, "%zu%s", id, extension);

        const int save_options =
            XML_SAVE_FORMAT | XML_SAVE_NO_DECL | XML_SAVE_NO_EMPTY | XML_SAVE_AS_HTML;
        xmlSaveCtxt* saveCtxt = xmlSaveToFilename(filename, "UTF-8", save_options);
        xmlSaveDoc(saveCtxt, doc);
        xmlSaveClose(saveCtxt);

        FLB_LOG_INFO("Saved thread %zu to file '%s'", id, filename);
    }

    download_thread_cleanup(memory.data, doc, context, resources_tree);
    return 0;
}

static void curl_all_cleanup(CURL* curl_handle) {
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
}

int flb_download_threads(size_t start, size_t end) {
    const useconds_t interval = 150 * 1000;  // 150 ms

    FLB_LOG_INFO_LB("Using interval %d microseconds", interval);

    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl_handle = curl_easy_init();

    if (!curl_handle) {
        FLB_LOG_ERROR("Can't init cURL handle");
        curl_all_cleanup(curl_handle);
        return 1;
    }

    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, kFirefoxUserAgent);

    FLB_LOG_INFO("Using User Agent '%s'", kFirefoxUserAgent);

    for (size_t id = start; id <= end; ++id) {
        if (flb_download_thread(curl_handle, id) != 0) {
            FLB_LOG_ERROR("Can't download thread %zu, exiting...", id);
            curl_all_cleanup(curl_handle);
            return 1;
        }

        usleep(interval);
    }

    curl_all_cleanup(curl_handle);
    return 0;
}
