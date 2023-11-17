#include "memory/memory.h"

#include <stdlib.h>
#include <string.h>

#include "logger/logger.h"

size_t flb_write_memory_callback(const void* contents, size_t size, size_t nmemb,
                                 flb_memstruct_t* memory) {
    if (!contents || !memory) {
        FLB_LOG_ERROR("Contents = %p, userdata = %p", contents, memory);
        return 0;
    }

    const size_t real_size = size * nmemb;

    if (memory->allocated_bytes - 1 - memory->data_len < real_size) {
        const size_t new_size = memory->allocated_bytes + real_size * 8 + 1;
        char* ptr = (char*) realloc(memory->data, new_size);

        if (!ptr) {
            FLB_LOG_ERROR("Can not reallocate memory data");
            return 0;
        }

        memory->data = ptr;
        memory->allocated_bytes = new_size;
    }

    memcpy(memory->data + memory->data_len, contents, real_size);
    memory->data_len += real_size;
    memory->data[memory->data_len] = '\0';

    return real_size;
}

size_t flb_write_file_callback(const void* contents, size_t size, size_t nmemb, FILE* fd) {
    if (!contents || !fd) {
        FLB_LOG_ERROR("Pointer = %p, file descriptor = %p", contents, fd);
        return 0;
    }

    return fwrite(contents, size, nmemb, fd);
}
