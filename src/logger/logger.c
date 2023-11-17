#include "logger/logger.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

static size_t calculate_buffer_size(const char* format, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);

    const size_t size = vsnprintf(NULL, 0, format, args_copy) + 1;

    va_end(args_copy);
    return size;
}

void flb_logger(const char* category, const char* function, const char* format, ...) {
    if (format) {
        va_list args;
        va_start(args, format);

        const size_t buffer_size = calculate_buffer_size(format, args);
        char buffer[buffer_size];

        vsnprintf(buffer, buffer_size, format, args);
        va_end(args);

        printf("%s> %s%s()%s: %s\n", category, LOGCLR_GREEN, function, LOGCLR_NORMAL, buffer);
        return;
    }

    printf("%s> %s%s()%s: Unknown log message\n", category, LOGCLR_GREEN, function, LOGCLR_NORMAL);
}
