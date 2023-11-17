#ifndef FLB_ARCHIVER_THREAD_H
#define FLB_ARCHIVER_THREAD_H

#include <curl/curl.h>
#include <libxml2/libxml/xpath.h>
#include <unistd.h>

int flb_download_threads(size_t start, size_t end, useconds_t interval);

#endif  // FLB_ARCHIVER_THREAD_H
