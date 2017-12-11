//
// Created by Toby Babu on 11/19/17.
//
#include <sys/fs.h>
#include <sys/allocator.h>
#include <sys/io.h>
#include <sys/tarfs.h>
#include <sys/klibc.h>
#include <sys/syscall.h>

struct filesys_tnode *
find_file_from_root(struct filesys_tnode *current_node, char file_name[10][256], int depth, int total_depth) {
    struct filesys_node *current_inode = current_node->link_to_inode;
    struct filesys_tnode *ret;
    if (kstrcmp(current_node->name, file_name[depth]) == 0) {
        if (depth == total_depth)
            return current_node;
        else {
            if ((current_inode->flags == FS_DIRECTORY) || (current_inode->flags == FS_MOUNTPOINT)) {
                if (current_inode->num_sub_directory > 0) {
                    for (int i = 0; i < current_inode->num_sub_directory; i++) {
                        ret = find_file_from_root(&current_inode->sub_directory_list[i], file_name, depth + 1,
                                                  total_depth);
                        if (ret != NULL)
                            return ret;
                    }
                }
                if (current_inode->num_sub_files > 0) {
                    for (int i = 0; i < current_inode->num_sub_files; i++) {
                        ret = find_file_from_root(&current_inode->sub_files_list[i], file_name, depth + 1, total_depth);
                        if (ret != NULL)
                            return ret;
                    }
                }
            }
            if (current_inode->flags == FS_FILE) {
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

struct filesys_tnode *find_file(char *file_name) {
    if (file_name[0] == '/') {
        // Search from root
        char sub_name[10][256];
        int i = 0, j = 0, k = 0, len = 0;
        len = kstrlength(file_name);
        for (i = 0, j = 0, k = 0; i < len; i++, j++) {
            sub_name[k][j] = file_name[i];
            if (file_name[i] == '/') {
                sub_name[k][j + 1] = '\0';
                if (i != len - 1) {
                    k++;
                    j = -1;
                    memset(sub_name[k], 0, 256);
                }

            }
        }
        sub_name[k][j + 1] = '\0';
        struct filesys_tnode *found_file = find_file_from_root(filefs_root, sub_name, 0, k);
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

int read_vfs(struct filesys_tnode *node, char *buf, int size) {
    if (node->link_to_inode->read != 0) {
        uint64_t position = node->link_to_inode->starting_position + node->link_to_inode->offset;
        uint64_t end = node->link_to_inode->starting_position + node->size;
        if (position >= end)
            return -1;
        if (size > node->size) {
            size = (int) end - position;
        }
        int retval = node->link_to_inode->read(buf, size, position);
        node->link_to_inode->offset = node->link_to_inode->offset + retval;
        return retval;
    }
    return 0;
}

int do_fread(char *buf, int size, uint64_t position) {
    kmemcpychar((void *) position, (void *) buf, size);
    return size;
}

int write_vfs(struct filesys_tnode *node, char *buf, int size, int offset) {
    if (node->link_to_inode->write != 0) {
        int retval = node->link_to_inode->write(buf, size, offset);
        return retval;
    }
    return 0;
}

struct filesys_tnode *copytnode(struct filesys_tnode *item) {
    struct filesys_tnode *current_tnode = bump(sizeof(struct filesys_tnode));
    kstrcopy(current_tnode->name, item->name);
    current_tnode->size = item->size;
    struct filesys_node *current_node = bump(sizeof(struct filesys_node));
    current_node->offset = item->link_to_inode->offset;
    current_node->starting_position = item->link_to_inode->starting_position;
    current_node->write = item->link_to_inode->write;
    current_node->flags = item->link_to_inode->flags;
    current_node->num_sub_directory = 0;
    current_node->num_sub_files = 0;
    current_node->sub_directory_list = 0;
    current_node->sub_files_list = 0;
    current_node->open = item->link_to_inode->open;
    current_node->perms = item->link_to_inode->perms;
    current_node->read = item->link_to_inode->read;
    current_tnode->link_to_inode = current_node;
    return current_tnode;

}

int do_dopen(char *file_name) {
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

int do_fopen(char *file_name) {
    struct filesys_tnode *item = find_file(file_name);
    if (item == NULL) {
        //kprintf("\n File not found");
        return -1;
    }
    struct filesys_tnode *copyitem;
    copyitem = copytnode(item);

    for (int i = 3; i < 100; i++) {
        if (file_descriptors[i] == NULL) {
            file_descriptors[i] = copyitem;
            return i;
        }
    }
    return -1;
}

int do_findfile(char *filename) {
    struct filesys_tnode *item = find_file(filename);
    if (item == NULL) {
        // kprintf("\n File not found");
        return -1;
    }
    return 0;
}

DIR *do_opendir(char *dirname) {
    DIR *dirptr = NULL;
    int fd, i, j;
    fd = do_dopen(dirname);
    struct filesys_node *inode = file_descriptors[fd]->link_to_inode;
    int totalfiles = inode->num_sub_files + inode->num_sub_directory;
    dirptr = (DIR *) sys_malloc(totalfiles * sizeof(DIR));
    map_user_address((uint64_t) dirptr, (uint64_t) dirptr - USERBASE, 4096,
                     (struct pml4t *) ((uint64_t) currentthread->page_table + KERNBASE), 7);
    memset(dirptr, 0, totalfiles * sizeof(DIR));
    for (i = 0; i < inode->num_sub_directory; i++) {
        kstrcopy(dirptr[i].d_name, inode->sub_directory_list[i].name);
        dirptr[i].d_no = 0;
        dirptr[i].fd = fd;
    }
    for (j = 0; j < inode->num_sub_files; j++) {
        kstrcopy(dirptr[i].d_name, inode->sub_files_list[j].name);
        dirptr[i].d_no = 0;
        dirptr[i].fd = fd;
        if (inode->sub_files_list[j].link_to_inode->link_to_process != NULL) {
            kstrcopy(dirptr[i].d_name, inode->sub_files_list[j].link_to_inode->link_to_process->name);
            if (inode->sub_files_list[j].link_to_inode->link_to_process->state == 0)
                dirptr[i].state = 0;
            else if (inode->sub_files_list[j].link_to_inode->link_to_process->state == 1)
                dirptr[i].state = 1;
            else if (inode->sub_files_list[j].link_to_inode->link_to_process->state == 2)
                dirptr[i].state = 2;
            dirptr[i].pid = inode->sub_files_list[j].link_to_inode->link_to_process->pid;
            dirptr[i].ppid = inode->sub_files_list[j].link_to_inode->link_to_process->ppid;
            //dirptr[i].state = inode->sub_files_list[j].link_to_inode->link_to_process->state;
        }
        i++;
    }
    return dirptr;
}

struct dirent *do_readdir(DIR *dir) {
    struct dirent *file = NULL;
    int i;
    struct filesys_node *inode = file_descriptors[dir->fd]->link_to_inode;
    int totalfiles = inode->num_sub_files + inode->num_sub_directory;
    for (i = 0; dir[i].d_no == 1; i++);

    if ((i < totalfiles) && (kstrlength(dir[i].d_name) > 0)) {
        file = dir + i;
        dir[i].d_no = 1;
    }
    return file;
}

int do_closedir(DIR *dirp) {
    for (int i = 0; i < 100; i++) {
        if (i == dirp->fd) {
            file_descriptors[i] = 0;
            //free dirp pointer as well
            for (int j = 0; kstrlength(dirp[j].d_name) > 0; j++) {
                memset(dirp[j].d_name, 0, sizeof(dirp[j].d_name));
                dirp[j].d_no = -1;
                dirp[j].fd = -1;
            }
            return 0;
        }
    }
    return -1;
}

int do_close(int fd) {
    for (int i = 0; i < 100; i++) {
        if (i == fd) {
            file_descriptors[i] = 0;
            return 0;
        }

    }

    return -1;
}

int do_write_err(char *buf, int size, int offset) {
    char value[2];
    for (int i = 0; i < size; i++) {
        value[0] = *(buf + i);
        value[1] = '\0';
        kprintf(value);
    }
    return 1;
}

int do_write(char *buf, int size, int offset) {
    if (size <= 0) {
        return -1;
    }
    char value[size];
    kmemcpychar(buf, value, size);
    buf[size] = '\0';
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
    for (int i = 0; i < size; i++) {
        buf[i] = *((uint64_t *) position + i);
    }
    return 0;
}

int check_for_ip(char *ab, int size_ip, uint64_t position) {
    if (size_ip == 0) {
        while (1) {
            int size = get_terminal_size();
            char *buf = get_terminal_buf();
            if (*(buf + size - 1) == '\n') {
                kmemcpychar(buf, ab, size);
                reset_terminal();
                return size;
            }
        }
    } else {
        while (1) {
            int size = get_terminal_size();
            if (size == size_ip) {
                char *buf = get_terminal_buf();
                ab = kstrncopy(ab, buf, size_ip);
                reset_terminal();
                return size_ip;
            }
        }
    }
}

struct filesys_node *create_sys_node() {
    struct filesys_node *current_node = bump(sizeof(struct filesys_node));
    current_node->write = do_write;
    current_node->sub_directory_list = NULL;
    current_node->num_sub_directory = 0;
    current_node->sub_files_list = NULL;
    current_node->num_sub_files = 0;
    return current_node;
}

struct filesys_tnode *create_t_node(char name[256]) {
    struct filesys_tnode *current_node = bump(sizeof(struct filesys_tnode));
    int i = 0;
    for (i = 0; i < kstrlength(name); i++) {
        current_node->name[i] = name[i];
    }
    current_node->name[i] = '\0';
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

struct filesys_tnode *file_exist(struct filesys_node *current_node, int file_num, char *file_name) {
    if (file_num == 0) {
        return NULL;
    }

    for (int i = 0; i < file_num; i++) {
        struct filesys_tnode *sub_file = &current_node->sub_files_list[i];
        if (sub_file->name == file_name) {
            return sub_file;
        }
    }
    return NULL;
}

struct filesys_tnode *folder_exist(struct filesys_node *current_node, int folder_num, char *folder_name) {
    if (folder_num == 0) {
        return NULL;
    }

    for (int i = 0; i < folder_num; i++) {
        struct filesys_tnode *sub_folder = &current_node->sub_directory_list[i];
        if (kstrcmp(sub_folder->name, folder_name) == 0) {
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

void load_tarfs(struct filesys_tnode *current_node) {
    struct posix_header_ustar *tarfs;
    uint64_t size;
    //kprintf("size of : %d",sizeof(struct posix_header_ustar));
    uint64_t address = (uint64_t) &_binary_tarfs_start;
    tarfs = (struct posix_header_ustar *) address;
    struct filesys_tnode *active_node = current_node;
    while (tarfs->name[0] != '\0') {
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
            if (sub_name[l][length - 1] == '/') {
                //sub_name[i + 1] = '\0';
                struct filesys_tnode *exist_node = folder_exist(active_node->link_to_inode,
                                                                active_node->link_to_inode->num_sub_directory,
                                                                sub_name[l]);
                if (exist_node == NULL) {
                    struct filesys_tnode *new_node = create_t_node(sub_name[l]);
                    new_node->link_to_inode->flags = FS_DIRECTORY;
                    new_node->size = size;
                    if (active_node->link_to_inode->num_sub_directory == 0) {
                        active_node->link_to_inode->sub_directory_list = new_node;
                    } else {
                        active_node->link_to_inode->sub_directory_list[active_node->link_to_inode->num_sub_directory] = *new_node;
                    }
                    active_node->link_to_inode->num_sub_directory++;
                    active_node = new_node;
                } else {
                    active_node = exist_node;
                }
            } else {
                struct filesys_tnode *exist_node = file_exist(active_node->link_to_inode,
                                                              active_node->link_to_inode->num_sub_directory,
                                                              sub_name[l]);
                if (exist_node == NULL) {
                    struct filesys_tnode *new_node = create_t_node(sub_name[l]);
                    new_node->link_to_inode->flags = FS_FILE;
                    new_node->link_to_inode->read = do_fread;
                    new_node->size = size;
                    new_node->link_to_inode->starting_position = (uint64_t) tarfs + sizeof(struct posix_header_ustar);
                    new_node->link_to_inode->offset = 0;
                    if (active_node->link_to_inode->num_sub_files == 0) {
                        active_node->link_to_inode->sub_files_list = new_node;
                    } else {
                        active_node->link_to_inode->sub_files_list[active_node->link_to_inode->num_sub_files] = *new_node;
                    }
                    active_node->link_to_inode->num_sub_files++;
                    active_node = new_node;
                } else {
                    active_node = exist_node;
                }
            }
        }

        //kprintf("subname name: %s ",sub_name[0]);
        /*kprintf("name: %s ",tarfs->name);
        kprintf("size: %d",size);
        kprintf("typeflag: %s\n",tarfs->typeflag);*/
        address += ((size / 512) + 1) * 512;
        tarfs = (struct posix_header_ustar *) address;
        if (size % 512) {
            address += 512;
            tarfs = (struct posix_header_ustar *) address;
        }
    }
}

void initialise_tarfs() {
    struct filesys_tnode *rootfs_directory = create_t_node("rootfs/");
    filefs_root->link_to_inode->sub_directory_list[filefs_root->link_to_inode->num_sub_directory] = *rootfs_directory;
    filefs_root->link_to_inode->num_sub_directory++;
    rootfs_directory->link_to_inode->flags = FS_MOUNTPOINT;
    load_tarfs(rootfs_directory);

}

void initialise_file_system() {
    filefs_root = create_t_node("/");
    struct filesys_tnode *dev_directory = create_t_node("dev/");
    proc_directory = create_t_node("proc/");
    struct filesys_tnode *terminal = create_t_node("ttyl");
    terminal->link_to_inode->flags = FS_DIRECTORY;
    filefs_root->link_to_inode->sub_directory_list = dev_directory;
    filefs_root->link_to_inode->num_sub_directory++;
    filefs_root->link_to_inode->flags = FS_MOUNTPOINT;
    filefs_root->link_to_inode->sub_directory_list[filefs_root->link_to_inode->num_sub_directory] = *proc_directory;
    filefs_root->link_to_inode->num_sub_directory++;
    dev_directory->link_to_inode->sub_files_list = terminal;
    dev_directory->link_to_inode->num_sub_files++;
    dev_directory->link_to_inode->flags = FS_MOUNTPOINT;

    struct filesys_tnode *std_out = create_t_node("stdout");
    std_out->link_to_inode->flags = FS_FILE;
    std_out->link_to_inode->write = &do_write;
    struct filesys_tnode *std_in = create_t_node("stdin");
    std_in->link_to_inode->flags = FS_FILE;
    std_out->link_to_inode->read = &check_for_ip;
    std_out->link_to_inode->write = NULL;
    struct filesys_tnode *std_err = create_t_node("stderr");
    std_err->link_to_inode->flags = FS_FILE;
    std_err->link_to_inode->write = &do_write_err;
    terminal->link_to_inode->sub_files_list = std_in;
    terminal->link_to_inode->num_sub_files++;
    terminal->link_to_inode->sub_files_list[1] = *std_out;
    terminal->link_to_inode->num_sub_files++;
    terminal->link_to_inode->sub_files_list[2] = *std_err;
    terminal->link_to_inode->num_sub_files++;
    file_descriptors[0] = std_in;
    file_descriptors[1] = std_err;
    file_descriptors[2] = std_err;
    initialise_tarfs();

    //for testing
    // print_path(filefs_root);
    proc_directory->link_to_inode->flags = FS_MOUNTPOINT;
    /* DIR *dir = do_opendir("/rootfs/bin/");
     DIR *dirent;
     dirent=do_readdir(dir);
     kprintf(dirent->d_name);
     dirent=do_readdir(dir);
     kprintf(dirent->d_name);
     do_closedir(dir);*/
}






