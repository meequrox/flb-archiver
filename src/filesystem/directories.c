#include "filesystem/directories.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

int flb_mkdir(const char* path) {
#if defined(_WIN32)
    int rc = mkdir(path);
#else
    int rc = mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif

    return rc;
}

int flb_mkdirs(char* path) {
    if (!path) {
        return 1;
    }

    const char sep = '/';

    char* slash_pos = strchr(path, sep);
    while (slash_pos) {
        *slash_pos = '\0';

        if (flb_mkdir(path) != 0) {
            perror(path);
            return 1;
        }

        *slash_pos = sep;
        slash_pos = strchr(slash_pos + 1, sep);
    }

    return 0;
}

int flb_chdir_out(void) {
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);

    char dirname_buf[32] = {'\0'};
    strftime(dirname_buf, sizeof(dirname_buf), "flb_%y.%m.%d_%H-%M", tm);

    if (flb_mkdir(dirname_buf) != 0) {
        return 1;
    }

    if (chdir(dirname_buf) != 0) {
        perror(dirname_buf);
        return 1;
    }

    return 0;
}
