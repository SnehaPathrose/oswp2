//
// Created by Toby Babu on 11/19/17.
//

#ifndef COURSEPROJ_FS_H
#define COURSEPROJ_FS_H

#include "defs.h"
#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08

struct filesys_tnode {
    char name[256];
    uint64_t size;
    struct filesys_node *link_to_inode;
}*file_descriptors[100], *filefs_root;

struct filesys_node {
    uint32_t perms;
    uint32_t flags;
    uint64_t starting_position;
    struct filesys_tnode *sub_directory_list;
    struct filesys_tnode *sub_files_list;
    int num_sub_directory;
    int num_sub_files;
    int (*read)(uint64_t *buf, int size, int offset, uint64_t position);
    int (*write)(char *buf, int size, int offset);
    int (*open)(char *file_name);
};

int read_vfs(struct filesys_node *node, uint64_t *buf, int size, int offset);
int write_vfs(struct filesys_node *node, char *buf, int size, int offset);
void initialise_file_system();


#endif //COURSEPROJ_FS_H
