#include "filesystem/directories.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "logger/logger.h"

int flb_mkdir(const char* path) {
#if defined(_WIN32)
    int rc = mkdir(path);
#else
    int rc = mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif

    if (rc == 0) {
        FLB_LOG_INFO("Created directory '%s'", path);
    }

    return rc;
}

int flb_mkdirs(const char* path) {
    if (!path) {
        return 1;
    }

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
            FLB_LOG_ERROR("%s: %s", path_copy, strerror(errno));
        }

        *slash_pos = sep;
        slash_pos = strchr(slash_pos + 1, sep);
    }

    free(path_copy);
    return failed;
}

int flb_chdir_out(void) {
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);

    char dirname_buf[32] = {'\0'};
    strftime(dirname_buf, sizeof(dirname_buf), "flb_%y.%m.%d_%H-%M", tm);

    if ((flb_mkdir(dirname_buf) != 0 && errno != EEXIST) || chdir(dirname_buf) != 0) {
        FLB_LOG_ERROR("%s: %s", dirname_buf, strerror(errno));
        return 1;
    }

    FLB_LOG_INFO("Current working directory: '%s'", dirname_buf);

    return 0;
}
