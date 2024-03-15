#include <inttypes.h>
#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "download/thread.h"
#include "filesystem/directories.h"
#include "logger/logger.h"
#include "version.h"

void usage(char** argv) {
    printf("Usage: %s START_ID END_ID\n", argv[0]);
    printf("Example: %s 151 1337\n", argv[0]);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        usage(argv);
        return EXIT_FAILURE;
    }

#if defined(_WIN32)
    // Terminal color fix
    system("");  // NOLINT
#endif

#if defined(NDEBUG)
    FLB_LOG_INFO("Release build " FLB_VERSION " " __DATE__ " " __TIME__);
#else
    FLB_LOG_INFO("Debug build " FLB_VERSION " " __DATE__ " " __TIME__);
#endif

    const int start = strtol(argv[1], NULL, 10);
    const int end = strtol(argv[2], NULL, 10);

    if (start < 1 || end < 1 || end < start) {
        FLB_LOG_ERROR("Invalid ID range [%d, %d]\n", start, end);

        usage(argv);
        return EXIT_FAILURE;
    }

    FLB_LOG_INFO("ID range [%d; %d]", start, end);

    const size_t interval_ms = 150;
    const size_t ms_to_us_multiplier = 1000;
    FLB_LOG_INFO("Using interval %" PRIuPTR " ms", interval_ms);

    extern const char kBaseUrl[];
    extern const char kFirefoxUserAgent[];
    FLB_LOG_INFO("Using base URL '%s'", kBaseUrl);
    FLB_LOG_INFO("Using user agent '%s'", kFirefoxUserAgent);

    extern const char kTimeLocale[];
    setlocale(LC_TIME, kTimeLocale);
    FLB_LOG_INFO("Using time locale '%s'", kTimeLocale);

    flb_chdir_out();
    flb_download_threads(start, end, interval_ms * ms_to_us_multiplier);

    return EXIT_SUCCESS;
}
