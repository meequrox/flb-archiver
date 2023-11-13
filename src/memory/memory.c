#include "memory/memory.h"

#include <stdlib.h>
#include <string.h>

size_t flb_write_memory_callback(const void* contents, size_t size, size_t nmemb,
                                 flb_memstruct_t* userdata) {
    if (!userdata) {
        fprintf(stderr, "%s: userdata is NULL\n", __func__);
        return 0;
    }

    size_t realsize = size * nmemb;
    char* ptr = realloc(userdata->data, userdata->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "%s: not enough memory\n", __func__);
        return 0;
    }

    userdata->data = ptr;
    memcpy(&(userdata->data[userdata->size]), contents, realsize);
    userdata->size += realsize;
    userdata->data[userdata->size] = 0;

    return realsize;
}

size_t flb_write_file_callback(const char* ptr, size_t size, size_t nmemb, FILE* fd) {
    if (!fd) {
        fprintf(stderr, "%s: file descriptor is NULL\n", __func__);
        return 0;
    }

    return fwrite(ptr, size, nmemb, fd);
}
