#ifndef FLB_ARCHIVER_COMMENTS_H
#define FLB_ARCHIVER_COMMENTS_H

#include <curl/curl.h>
#include <libxml2/libxml/xpath.h>

int include_comments(CURL* curl_handle, xmlXPathContext* context);

#endif  // FLB_ARCHIVER_COMMENTS_H
