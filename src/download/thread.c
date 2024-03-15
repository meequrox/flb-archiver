#include "download/thread.h"

#if defined(_WIN32)
    #include <windows.h>
#endif

#include <curl/curl.h>
#include <curl/easy.h>
#include <errno.h>
#include <inttypes.h>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlsave.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "data_structures/linked_list.h"
#include "download/comments.h"
#include "filesystem/directories.h"
#include "logger/logger.h"
#include "memory/memory.h"

const char kBaseUrl[] = "https://flareboard.ru/";
const size_t kBaseUrlLen = sizeof(kBaseUrl) / sizeof(*kBaseUrl) - 1;

const char kFirefoxUserAgent[] =
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:99.0) Gecko/20100101 Firefox/99.0";

const char kTimeLocale[] = "en_US.UTF-8";

static int page_is_thread(xmlXPathContext* context) {
    // Expected != NULL: context

    const xmlChar* expr = (xmlChar*) "(//div[@class='postTop'])[1]";
    xmlXPathObject* result = xmlXPathEvalExpression(expr, context);
    int nodes = result->nodesetval->nodeNr;

    xmlXPathFreeObject(result);
    return nodes;
}

static void convert_timestamp(const char* timestamp_str, char* dest, size_t dest_size) {
    // Expected != NULL: timestamp_str, dest

    const int timestamp_base = 10;
    time_t timestamp = strtoull(timestamp_str, NULL, timestamp_base);

    struct tm tm;
    struct tm* tm_ptr = &tm;

#if defined(_WIN32)
    gmtime_s(&tm, &timestamp);
#else
    tm_ptr = gmtime_r(&timestamp, &tm);
#endif

    strftime(dest, dest_size, "%d %b %Y %H:%M", tm_ptr);
}

static int fix_timestamps(xmlXPathContext* context) {
    // Expected != NULL: context

    const xmlChar* expr = (xmlChar*) "//div[@class='time']";
    xmlXPathObject* result = xmlXPathEvalExpression(expr, context);
    xmlNodeSet* nodes = result->nodesetval;

    for (int i = 0; i < nodes->nodeNr; ++i) {
        xmlNode* node = nodes->nodeTab[i];
        char* timestamp_str = (char*) xmlNodeGetContent(node);

        if (timestamp_str) {
            const size_t buf_size = 32;
            char buf[buf_size];

            convert_timestamp(timestamp_str, buf, buf_size);
            char* buf_ptr = buf[0] == ' ' ? buf + 1 : buf;

            xmlNodeSetContent(node, (xmlChar*) buf_ptr);
            xmlFree(timestamp_str);
        }
    }

    xmlXPathFreeObject(result);
    return 0;
}

static void save_resource_setup(CURL* curl_handle, const char* url, void* file) {
    // Expected != NULL: curl_handle, url, file

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 10);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, flb_write_file_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, file);
}

static int save_resource(CURL* curl_handle, const char* url, const char* filename) {
    // Expected != NULL: curl_handle, url

    if (!filename) {
        return 1;
    }

    if (access(filename, F_OK) == 0) {
        return 0;
    }

    flb_mkdirs(filename);

#if defined(_WIN32)
    const int is_binary_file =
        strstr(filename, "img/") == filename || strstr(filename, "media/") == filename;
#else
    const int is_binary_file = 0;
#endif

    FILE* file = NULL;

    if (is_binary_file) {
        file = fopen(filename, "wbx");
    } else {
        file = fopen(filename, "wx");
    }

    if (!file) {
        char errbuf[128] = {0};

#if defined(_WIN32)
        strerror_s(errbuf, sizeof(errbuf), errno);
#else
        strerror_r(errno, errbuf, sizeof(errbuf));
#endif

        FLB_LOG_ERROR("%s: %s", filename, errbuf);
        return errno != EEXIST;
    }

    save_resource_setup(curl_handle, url, file);
    CURLcode response = curl_easy_perform(curl_handle);
    fclose(file);

    if (response != CURLE_OK) {
        unlink(filename);

        FLB_LOG_ERROR("Can't fetch '%s': %s", url, curl_easy_strerror(response));
        return 1;
    }

    FLB_LOG_INFO("Created new resource file: %s", filename);
    return 0;
}

static int is_local_path(const char* path) {
    return path != strstr(path, "https://") && path != strstr(path, "http://");
}

static flb_list_node* parse_resources(flb_list_node* list, xmlXPathContext* context,
                                      const char* xpath_expr, const char* find_attr) {
    // Expected != NULL: context, xpath_expr, find_attr

    xmlXPathObject* result = xmlXPathEvalExpression((xmlChar*) xpath_expr, context);
    xmlNodeSet* nodes = result->nodesetval;

    for (int i = 0; i < nodes->nodeNr; ++i) {
        char* attr = (char*) xmlGetProp(nodes->nodeTab[i], (xmlChar*) find_attr);

        if (attr && is_local_path(attr)) {
            const size_t resource_url_size = kBaseUrlLen + strlen(attr) + 1;
            char resource_url[resource_url_size];
            snprintf(resource_url, resource_url_size, "%s%s", kBaseUrl, attr);

            list = flb_list_insert_front(list, resource_url, attr);

            xmlFree((xmlChar*) attr);
        }
    }

    xmlXPathFreeObject(result);
    return list;
}

static void download_thread_page_cleanup(char* data, xmlDoc* doc, xmlXPathContext* context,
                                         flb_list_node* list) {
    free(data);
    xmlFreeDoc(doc);
    xmlXPathFreeContext(context);
    flb_list_free(list);
}

static void save_thread(size_t id, size_t max_id_len, xmlDoc* doc) {
    // Expected != NULL: doc

    const char extension[] = ".html";

    const size_t filename_size = max_id_len + sizeof(extension) / sizeof(*extension);
    char filename[filename_size];

    snprintf(filename, filename_size, "%" PRIuPTR "%s", id, extension);

    const int save_options =
        XML_SAVE_FORMAT | XML_SAVE_NO_DECL | XML_SAVE_NO_EMPTY | XML_SAVE_AS_HTML;  // NOLINT
    xmlSaveCtxt* saveCtxt = xmlSaveToFilename(filename, "UTF-8", save_options);

    xmlSaveDoc(saveCtxt, doc);
    xmlSaveClose(saveCtxt);

    FLB_LOG_INFO("Saved thread %" PRIuPTR " to file '%s'", id, filename);
}

static void download_thread_page_setup(CURL* curl_handle, const char* url, void* memory, int mode) {
    // Expected != NULL: curl_handle
    // mode: 0 - Set options to download thread without reset
    //       1 - Reset then set UA
    //       2 - Reset then set UA and options to download thread

    switch (mode) {
        case 1:
            curl_easy_reset(curl_handle);
            curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 8);
            curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, kFirefoxUserAgent);
            break;
        case 2:
            curl_easy_reset(curl_handle);
            curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 8);
            curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, kFirefoxUserAgent);
            // fallthrough
        case 0:
            curl_easy_setopt(curl_handle, CURLOPT_URL, url);
            curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 0);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, flb_write_memory_callback);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, memory);
        default:
            break;
    }
}

static int download_thread_page(CURL* curl_handle, size_t id) {
    // Expected != NULL: curl_handle

    const char query_base[] = "thread.php?id=";
    const size_t query_base_size = sizeof(query_base) / sizeof(*query_base);

    const size_t uint_max_digits = 10;
    const size_t thread_url_size = kBaseUrlLen + uint_max_digits + query_base_size;

    char thread_url[thread_url_size];

    snprintf(thread_url, thread_url_size, "%s%s%" PRIuPTR, kBaseUrl, query_base, id);

    const size_t initial_data_size = 256 * 1024 + 1;
    flb_memstruct_t memory = {(char*) malloc(initial_data_size), 0, initial_data_size};

    download_thread_page_setup(curl_handle, thread_url, &memory, 0);
    CURLcode response = curl_easy_perform(curl_handle);

    if (response != CURLE_OK) {
        FLB_LOG_ERROR("Can't fetch %s: %s", thread_url, curl_easy_strerror(response));
        download_thread_page_cleanup(memory.data, NULL, NULL, NULL);
        return 1;
    }

    const int parse_options = HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING;  // NOLINT
    xmlDoc* doc = htmlReadDoc((xmlChar*) memory.data, NULL, NULL, parse_options);
    xmlXPathContext* context = xmlXPathNewContext(doc);

    if (!doc || !context) {
        FLB_LOG_ERROR("Can't read HTML document or create XPath context");
        download_thread_page_cleanup(memory.data, doc, context, NULL);
        return 1;
    }

    xmlXPathRegisterNs(context, (xmlChar*) "html", (xmlChar*) "http://www.w3.org/1999/xhtml");

    flb_list_node* resources_list = NULL;

    if (page_is_thread(context)) {
        FLB_LOG_INFO_LB("Processing thread %" PRIuPTR " (URL '%s')", id, thread_url);

        include_comments(curl_handle, context);
        download_thread_page_setup(curl_handle, NULL, NULL, 1);

        fix_timestamps(context);
        FLB_LOG_INFO("Fixed timestamps in thread %" PRIuPTR, id);

        resources_list = parse_resources(resources_list, context, "//link", "href");
        resources_list = parse_resources(resources_list, context, "//script", "src");
        resources_list = parse_resources(resources_list, context, "//img", "src");
        resources_list = parse_resources(resources_list, context, "//video", "src");

        if (resources_list) {
            flb_list_foreach3(resources_list, save_resource, curl_handle);
            FLB_LOG_INFO("Saved resources for thread %" PRIuPTR, id);
        }

        save_thread(id, uint_max_digits, doc);
    }

    download_thread_page_cleanup(memory.data, doc, context, resources_list);
    return 0;
}

static int get_cpu_count(void) {
#ifdef WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

typedef struct {
    size_t start;
    size_t end;
    useconds_t interval;
} WorkerOpts;

static void* flb_download_worker(void* arg) {
    WorkerOpts* opts = (WorkerOpts*) arg;
    FLB_LOG_INFO("Processing threads range [%" PRIuPTR "; %" PRIuPTR "] (interval %ld)", opts->start,
                 opts->end, opts->interval);

    CURL* curl_handle = curl_easy_init();
    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 8);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, kFirefoxUserAgent);

        for (size_t tid = opts->start; tid <= opts->end; ++tid) {
            if (download_thread_page(curl_handle, tid) != 0) {
                FLB_LOG_ERROR("Can't download thread %" PRIuPTR ", exiting...", tid);
                break;
            }

            usleep(opts->interval);
        }
    } else {
        FLB_LOG_ERROR("Can't init cURL handle");
    }

    curl_easy_cleanup(curl_handle);
    pthread_exit(NULL);
    return NULL;
}

int flb_download_threads(size_t start, size_t end, useconds_t interval) {
    curl_global_init(CURL_GLOBAL_ALL);

    const size_t flb_threads_count = end - start + 1;
    const int workers_count = flb_threads_count < (size_t) get_cpu_count() ? 1 : get_cpu_count();
    const size_t flb_threads_per_worker =
        workers_count == 1 ? flb_threads_count : flb_threads_count / workers_count;

    FLB_LOG_INFO("Workers: %d; FLB threads: %" PRIuPTR "; Threads per worker (avg): %" PRIuPTR,
                 workers_count, flb_threads_count, flb_threads_per_worker);

    pthread_t workers_ids[workers_count];
    WorkerOpts* workers_opts[workers_count];

    for (int w = 0; w < workers_count; ++w) {
        WorkerOpts* opts = malloc(sizeof(WorkerOpts));
        workers_opts[w] = opts;

        opts->start = start + w * flb_threads_per_worker;
        opts->end = start + (w + 1) * flb_threads_per_worker - 1;
        opts->interval = interval;

        if (w == workers_count - 1) {
            size_t rem = flb_threads_count % workers_count;
            opts->end += rem;
        }

        pthread_t ptid = 0;
        pthread_create(&ptid, NULL, flb_download_worker, opts);
        workers_ids[w] = ptid;
    }

    for (int w = 0; w < workers_count; ++w) {
        pthread_join(workers_ids[w], NULL);
    }

    for (int w = 0; w < workers_count; ++w) {
        free(workers_opts[w]);
    }

    curl_global_cleanup();
    return 0;
}
