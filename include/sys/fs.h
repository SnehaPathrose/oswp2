//
// Created by Toby Babu on 11/19/17.
//

#ifndef COURSEPROJ_FS_H
#define COURSEPROJ_FS_H

#include "defs.h"
#include "contextswitch.h"

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08

struct filesys_node {
    uint32_t perms;
    uint32_t flags;
    uint64_t starting_position;
    uint64_t offset;
    struct filesys_tnode *sub_directory_list;
    struct filesys_tnode *sub_files_list;
    struct PCB *link_to_process;
    int num_sub_directory;
    int num_sub_files;
    int (*read)(char *buf, int size, uint64_t position);
    int (*write)(char *buf, int size, int offset);
    int (*open)(char *file_name);
};

struct filesys_tnode {
    char name[256];
    uint64_t size;
    struct filesys_node *link_to_inode;
}*file_descriptors[100], *filefs_root, *proc_directory;


#define NAME_MAX 25

typedef struct dirent{
    char d_name[NAME_MAX+1];
    int d_no;
    int fd;
    int state;
    uint32_t pid, ppid;
}DIR;
DIR *do_opendir(char *dirname);
struct dirent *do_readdir(DIR *dir);
int do_closedir(DIR *dirp);
int do_close(int fd);
int do_findfile(char *filename);
int do_fopen(char *file_name);
struct filesys_tnode* find_file(char* file_name);
int do_open(char *file_name);
int read_vfs(struct filesys_tnode *node, char *buf, int size);
int write_vfs(struct filesys_tnode *node, char *buf, int size, int offset);
struct filesys_tnode * create_t_node(char name[256]);
void initialise_file_system();
int check_for_ip(char *ab, int size_ip, uint64_t position);


#endif //COURSEPROJ_FS_H




