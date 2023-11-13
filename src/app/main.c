#include <stdio.h>

#include "download/thread.h"
#include "filesystem/directories.h"

void usage(char** argv) {
    fprintf(stdout, "Usage: %s START_ID END_ID\n", argv[0]);
    fprintf(stdout, "Example: %s 151 1337\n", argv[0]);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        usage(argv);
        return EXIT_FAILURE;
    }

    const int start = strtol(argv[1], NULL, 10);
    const int end = strtol(argv[2], NULL, 10);

    if (start < 1 || end < 1 || end < start) {
        fprintf(stderr, "Invalid ID range [%d, %d]\n", start, end);
        usage(argv);
        return EXIT_FAILURE;
    }

    flb_chdir_out();
    flb_download_threads(start, end);

    return EXIT_SUCCESS;
}
