#ifndef FLB_ARCHIVER_MEMORY_H
#define FLB_ARCHIVER_MEMORY_H

#include <stddef.h>
#include <stdio.h>

typedef struct flb_memstruct {
    char* data;
    size_t size;
} flb_memstruct_t;

size_t flb_write_memory_callback(const void* contents, size_t size, size_t nmemb,
                                 flb_memstruct_t* userdata);
size_t flb_write_file_callback(const char* ptr, size_t size, size_t nmemb, FILE* fd);

#endif  // FLB_ARCHIVER_MEMORY_H
