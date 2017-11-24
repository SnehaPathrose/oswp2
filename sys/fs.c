//
// Created by Toby Babu on 11/19/17.
//
#include <sys/fs.h>
#include <sys/allocator.h>
#include <sys/io.h>
#include <sys/tarfs.h>
#include <sys/klibc.h>

struct filesys_tnode* find_file_from_root(struct filesys_tnode *current_node, char** file_name, int depth, int total_depth) {
    struct filesys_node *current_inode = current_node->link_to_inode;
    if (kstrcmp(current_node->name, file_name[depth]) == 0) {
        if (depth == total_depth)
            return current_node;
        else {
            if (current_inode->flags == FS_DIRECTORY) {
                if (current_inode->num_sub_directory > 0) {
                    for (int i = 0; i < current_inode->num_sub_directory; i++) {
                        depth++;
                        find_file_from_root(&current_inode->sub_directory_list[i], file_name, depth, total_depth);
                    }
                }
            }
            if(current_inode->flags == FS_FILE) {
                if ((current_inode->num_sub_files > 0) && (depth == total_depth)) {
                    for (int i = 0; i < current_inode->num_sub_files; i++) {
                        if (kstrcmp(current_node->name, file_name[depth]) == 0)
                            return &current_inode->sub_files_list[i];
                    }
                }
            }
        }
    }
    return NULL;
}

struct filesys_tnode* find_file(char* file_name) {
    if (file_name[0] == '/') {
        // Search from root
        char sub_name[10][256];
        int i = 0, j = 0, k = 0;
        for (i = 0, j = 0, k = 0; i < kstrlength(file_name); i++, j++) {
            sub_name[k][j] = file_name[i];
            if (file_name[i] == '/') {
                sub_name[k][j + 1] = '\0';
                k++;
                memset(sub_name[k], 0, 256);
                j = -1;
            }
        }
        sub_name[k][j + 1] = '\0';
        struct filesys_tnode *found_file = find_file_from_root(filefs_root, (char **) sub_name, 0, k);
        return found_file;
    }
    return NULL;
}

int open_vfs(struct filesys_node *node, char *file_name) {
    if (node->open != 0) {
        int retval = node->open(file_name);
        return retval;
    }
    return 0;
}

int read_vfs(struct filesys_node *node, uint64_t *buf, int size, int offset) {
    if (node->read != 0) {
        int retval = node->read(buf, size, offset, node->starting_position);
        return retval;
    }
    return 0;
}

int write_vfs(struct filesys_node *node, char *buf, int size, int offset) {
    if (node->write != 0) {
        int retval = node->write(buf, size, offset);
        return retval;
    }
    return 0;
}

int do_open(char *file_name) {
    struct filesys_tnode *item = find_file(file_name);
    if (item == NULL) {
        kprintf("\n File not found");
        return -1;
    }
    for (int i = 0; i < 100; i++) {
        if (file_descriptors[i] == NULL) {
            file_descriptors[i] = item;
            return i;
        }
    }
    return -1;
}

int do_write(char *buf, int size, int offset) {
    kprintf(buf);
    return 1;
}

int do_read(uint64_t *buf, int size, int offset, uint8_t keyscancode, uint64_t position) {
    /*int bufindex = 0;
    char buffer[1024], retval[3];
    kscanf(keyscancode, retval);
    while(*(retval + bufindex) != '\0') {
        buffer[bufindex] = retval[bufindex];
        bufindex++;
    }*/
    for (int i = 0; i < size; i++){
        buf[i] = *((uint64_t*)position + i);
    }
    return 0;
}

struct filesys_node * create_sys_node() {
    struct filesys_node *current_node = bump(sizeof(struct filesys_node));
    current_node->write = do_write;
    current_node->sub_directory_list = NULL;
    current_node->num_sub_directory = 0;
    current_node->sub_files_list = NULL;
    current_node->num_sub_files = 0;
    return current_node;
}

struct filesys_tnode * create_t_node(char name[256]) {
    struct filesys_tnode *current_node = bump(sizeof(struct filesys_tnode));
    for (int i = 0; i< kstrlength(name); i++) {
        current_node->name[i] = name[i];
    }
    current_node->link_to_inode = create_sys_node();
    //current_node->write = do_write;
    return current_node;
}

uint64_t get_size(const char *in) {
    uint64_t size = 0;
    unsigned int j;
    unsigned int count = 1;

    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);

    return size;
}

struct filesys_tnode * file_exist(struct filesys_node *current_node, int file_num, char* file_name) {
    if(file_num == 0) {
        return NULL;
    }

    for(int i = 0; i < file_num; i++) {
        struct filesys_tnode *sub_file = &current_node->sub_files_list[i];
        if(sub_file->name == file_name) {
            return sub_file;
        }
    }
    return NULL;
}

struct filesys_tnode * folder_exist(struct filesys_node *current_node, int folder_num, char* folder_name) {
    if(folder_num == 0) {
        return NULL;
    }

    for(int i = 0; i < folder_num; i++) {
        struct filesys_tnode *sub_folder = &current_node->sub_directory_list[i];
        if(kstrcmp(sub_folder->name, folder_name) == 0) {
            return sub_folder;
        }
    }
    return NULL;
}

void print_path(struct filesys_tnode *current_node) {
    struct filesys_node *current_inode = current_node->link_to_inode;
    if (current_inode->num_sub_directory > 0) {
        for (int i = 0; i < current_inode->num_sub_directory; i++) {
            kprintf(" %s -> %s \n", current_node->name, current_inode->sub_directory_list[i].name);
            print_path(&current_inode->sub_directory_list[i]);
        }
    }
    if (current_inode->num_sub_files > 0) {
        for (int i = 0; i < current_inode->num_sub_files; i++) {
            kprintf(" %s -> %s \n", current_node->name, current_inode->sub_files_list[i].name);
        }
    }
}

void load_tarfs(struct filesys_tnode *current_node)
{
    struct posix_header_ustar *tarfs;
    uint64_t size;
    kprintf("size of : %d",sizeof(struct posix_header_ustar));
    uint64_t address=(uint64_t)&_binary_tarfs_start;
    tarfs = (struct posix_header_ustar *)address;
    struct filesys_tnode *active_node = current_node;
    while(tarfs->name[0]!='\0')
    {
        active_node = current_node;
        size = get_size(tarfs->size);
        char sub_name[10][256];
        int i = 0, j = 0, k = 0;
        for (i = 0, j = 0, k = 0; i < kstrlength(tarfs->name); i++, j++) {
            sub_name[k][j] = tarfs->name[i];
            if (tarfs->name[i] == '/') {
                sub_name[k][j + 1] = '\0';
                k++;
                memset(sub_name[k], 0, 256);
                j = -1;
            }
        }
        sub_name[k][j + 1] = '\0';
        for (int l = 0; l <= k; l++) {
            int length = kstrlength(sub_name[l]);
            if (sub_name[l][0] == '\0')
                break;
            if(sub_name[l][length - 1] == '/') {
                //sub_name[i + 1] = '\0';
                struct filesys_tnode *exist_node = folder_exist(active_node->link_to_inode, active_node->link_to_inode->num_sub_directory, sub_name[l]);
                if (exist_node == NULL) {
                    struct filesys_tnode *new_node = create_t_node(sub_name[l]);
                    new_node->link_to_inode->flags = FS_DIRECTORY;
                    new_node->size = size;
                    if (active_node->link_to_inode->num_sub_directory == 0) {
                        active_node->link_to_inode->sub_directory_list = new_node;
                    }
                    else {
                        active_node->link_to_inode->sub_directory_list[active_node->link_to_inode->num_sub_directory] = *new_node;
                    }
                    active_node->link_to_inode->num_sub_directory++;
                    active_node = new_node;
                }
                else {
                    active_node = exist_node;
                }
            }
            else {
                struct filesys_tnode *exist_node = file_exist(active_node->link_to_inode, active_node->link_to_inode->num_sub_directory, sub_name[l]);
                if (exist_node == NULL) {
                    struct filesys_tnode *new_node = create_t_node(sub_name[l]);
                    new_node->link_to_inode->flags = FS_FILE;
                    new_node->size = size;
                    if (active_node->link_to_inode->num_sub_files == 0) {
                        active_node->link_to_inode->sub_files_list = new_node;
                    }
                    else {
                        active_node->link_to_inode->sub_files_list[active_node->link_to_inode->num_sub_files] = *new_node;
                    }
                    active_node->link_to_inode->num_sub_files++;
                    active_node = new_node;
                }
                else {
                    active_node = exist_node;
                }
            }
        }

        //kprintf("subname name: %s ",sub_name[0]);
        /*kprintf("name: %s ",tarfs->name);
        kprintf("size: %d",size);
        kprintf("typeflag: %s\n",tarfs->typeflag);*/
        address += ((size/512) + 1) * 512;
        tarfs = (struct posix_header_ustar *) address;
        if(size % 512) {
            address += 512;
            tarfs = (struct posix_header_ustar *) address;
        }
    }
}

void initialise_tarfs() {
    struct filesys_tnode *rootfs_directory = create_t_node("rootfs/");
    filefs_root->link_to_inode->sub_directory_list[filefs_root->link_to_inode->num_sub_directory] = *rootfs_directory;
    filefs_root->link_to_inode->num_sub_directory++;
    load_tarfs(rootfs_directory);

}

void initialise_file_system() {
    filefs_root = create_t_node("/");
    struct filesys_tnode *dev_directory = create_t_node("dev/");
    struct filesys_tnode *terminal = create_t_node("ttyl");
    filefs_root->link_to_inode->sub_directory_list = dev_directory;
    filefs_root->link_to_inode->num_sub_directory++;
    filefs_root->link_to_inode->flags = FS_MOUNTPOINT;
    dev_directory->link_to_inode->sub_files_list = terminal;
    dev_directory->link_to_inode->num_sub_files++;
    dev_directory->link_to_inode->flags = FS_MOUNTPOINT;
    file_descriptors[0] = terminal;
    file_descriptors[1] = terminal;
    file_descriptors[2] = terminal;
    initialise_tarfs();
    //print_path(filefs_root);
}
