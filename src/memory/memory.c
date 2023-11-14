#include "memory/memory.h"

#include <stdlib.h>
#include <string.h>

size_t flb_write_memory_callback(const void* contents, size_t size, size_t nmemb,
                                 flb_memstruct_t* userdata) {
    if (!userdata) {
        fprintf(stderr, "Userdata is NULL\n");
        return 0;
    }

    const size_t realsize = size * nmemb;

    if (userdata->allocated_bytes - 1 - userdata->data_len < realsize) {
        const size_t new_size = userdata->allocated_bytes + realsize * 8 + 1;
        char* ptr = (char*) realloc(userdata->data, new_size);

        if (!ptr) {
            fprintf(stderr, "Can not reallocate userdata memory\n");
            return 0;
        }

        userdata->data = ptr;
        userdata->allocated_bytes = new_size;
    }

    memcpy(&(userdata->data[userdata->data_len]), contents, realsize);
    userdata->data_len += realsize;
    userdata->data[userdata->data_len] = '\0';

    return realsize;
}

size_t flb_write_file_callback(const char* ptr, size_t size, size_t nmemb, FILE* fd) {
    if (!fd) {
        fprintf(stderr, "File descriptor is NULL\n");
        return 0;
    }

    return fwrite(ptr, size, nmemb, fd);
}
