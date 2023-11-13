#ifndef FLB_ARCHIVER_FILESYSTEM_H
#define FLB_ARCHIVER_FILESYSTEM_H

int flb_mkdir(const char* path);

int flb_mkdirs(char* path);

int flb_chdir_out(void);

#endif  // FLB_ARCHIVER_FILESYSTEM_H
