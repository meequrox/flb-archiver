#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdio.h>

struct MemoryStruct {
    char* data;
    size_t size;
};

size_t write_memory_callback(void* contents, size_t size, size_t nmemb, struct MemoryStruct* userdata);
size_t write_file_callback(char* ptr, size_t size, size_t nmemb, FILE* fd);

#endif  // MEMORY_H
