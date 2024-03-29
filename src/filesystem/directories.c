#include "filesystem/directories.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "logger/logger.h"

static int flb_mkdir(const char* path) {
    // Expected != NULL: path

#if defined(_WIN32)
    int rc = mkdir(path);
#else
    int rc = mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif

    return rc;
}

int flb_mkdirs(const char* path) {
    // Expected != NULL: path

    char* path_copy = strdup(path);
    if (!path_copy) {
        return 1;
    }

    const char sep = '/';

    char* slash_pos = strchr(path_copy, sep);
    int failed = 0;

    while (slash_pos && !failed) {
        *slash_pos = '\0';

        failed = flb_mkdir(path_copy) != 0 && errno != EEXIST;
        if (failed) {
            FLB_LOG_ERROR("%s: Can't create directory", path_copy);
        }

        *slash_pos = sep;
        slash_pos = strchr(slash_pos + 1, sep);
    }

    free(path_copy);
    return failed;
}

int flb_chdir_out(void) {
    const char dirname[] = "flb_archive";

    if (flb_mkdir(dirname) != 0 && errno != EEXIST) {
        FLB_LOG_ERROR("%s: Can't create directory", dirname);
        return 1;
    }

    if (chdir(dirname) != 0) {
        FLB_LOG_ERROR("%s: Can't change working directory", dirname);
        return 1;
    }

    FLB_LOG_INFO("Current working directory: %s", dirname);
    return 0;
}
