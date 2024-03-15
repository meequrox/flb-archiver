#include "logger/logger.h"

#include <inttypes.h>
#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#include "logger/colors.h"

static int calculate_buffer_size(const char* format, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);

    const int size = vsnprintf(NULL, 0, format, args_copy) + 1;

    va_end(args_copy);
    return size;
}

void flb_logger(const char* prompt, const char* function, const char* format, ...) {
    if (format) {
        va_list args;
        va_start(args, format);

        const int buffer_size = calculate_buffer_size(format, args);
        char buffer[buffer_size];

        vsnprintf(buffer, buffer_size, format, args);
        va_end(args);

        printf("%s %s[W%" PRIuPTR "] %s%s()%s: %s\n", prompt, LOGCLR_YELLOW, pthread_self(),
               LOGCLR_GREEN, function, LOGCLR_NORMAL, buffer);
        return;
    }

    printf("%s %s[W%" PRIuPTR "] %s%s()%s: Unknown log message\n", prompt, LOGCLR_YELLOW, pthread_self(),
           LOGCLR_GREEN, function, LOGCLR_NORMAL);
}
