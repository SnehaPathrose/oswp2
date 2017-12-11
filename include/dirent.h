#ifndef _DIRENT_H
#define _DIRENT_H

#include <sys/defs.h>

#define NAME_MAX 25

struct dirent {
    char d_name[NAME_MAX+1];
    int d_no;
    int fd;
    int state;
    uint32_t pid, ppid;
};

typedef struct dirent DIR;

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

#endif


