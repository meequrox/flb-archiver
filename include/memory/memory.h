#ifndef FLB_ARCHIVER_MEMORY_H
#define FLB_ARCHIVER_MEMORY_H

#include <stddef.h>
#include <stdio.h>

struct MemoryStruct {
    char* data;
    size_t size;
};

size_t write_memory_callback(const void* contents, size_t size, size_t nmemb,
                             struct MemoryStruct* userdata);
size_t write_file_callback(const char* ptr, size_t size, size_t nmemb, FILE* fd);

#endif  // FLB_ARCHIVER_MEMORY_H
