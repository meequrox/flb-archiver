#ifndef FLB_ARCHIVER_THREAD_H
#define FLB_ARCHIVER_THREAD_H

#include <curl/curl.h>
#include <libxml2/libxml/xpath.h>

int flb_is_thread(xmlXPathContext* context);

int flb_download_thread(CURL* curl_handle, size_t id);

int flb_download_threads(size_t start, size_t end);

#endif  // FLB_ARCHIVER_THREAD_H
