#include <stdio.h>

#include "download/thread.h"
#include "filesystem/directories.h"
#include "logger/logger.h"

void usage(char** argv) {
    printf("Usage: %s START_ID END_ID\n", argv[0]);
    printf("Example: %s 151 1337\n", argv[0]);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        usage(argv);
        return EXIT_FAILURE;
    }

#ifdef NDEBUG
    FLB_LOG_INFO("Release build");
#else
    FLB_LOG_INFO("Debug build");
#endif

    const int start = strtol(argv[1], NULL, 10);
    const int end = strtol(argv[2], NULL, 10);

    if (start < 1 || end < 1 || end < start) {
        FLB_LOG_ERROR("Invalid ID range [%d, %d]", start, end);

        usage(argv);
        return EXIT_FAILURE;
    }

    FLB_LOG_INFO("ID range [%d, %d]\n", start, end);

    flb_chdir_out();
    flb_download_threads(start, end);

    return EXIT_SUCCESS;
}
