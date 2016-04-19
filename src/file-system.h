#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__

#include <stdint.h>
#include <stdbool.h>
#include "kmem_cache.h"
#include "string.h"
#include "list.h"
#include "stdio.h"
#include "kernel.h"
#include "locking.h"

#define MAX_DIR_LEN      1024

#define FLAG_CREATE      (1 << 0)
#define FLAG_READ        (1 << 1)
#define FLAG_WRITE       (1 << 2)

struct file_desc {
    struct inode *file;
    struct spinlock file_lock;
    int flags;
};
typedef struct file_desc fd_t;

struct inode_holder {
    struct inode     *child;
    struct list_head  holders;
};
typedef struct inode_holder inode_holder_t;

struct inode {
    struct inode      *parent;
    char              *file_name;
    void              *data;
    size_t             size;
    bool               is_dir;
    struct list_head   dir_files; // not empty only for dirs
};
typedef struct inode inode_t;

void setup_fs(void);

/**
 * it creates full path to file and also a file.
 * is_dir only used if should_create = true
 */
fd_t *fs_open(const char *file_name, int flags);
void fs_close(fd_t *fd);
void fs_read(fd_t *fd, void *buf, size_t count);
void fs_write(fd_t *fd, const void *buf, size_t count);
void fs_mkdir(const char *path);
struct list_head* fs_readdir(const char *path);

//this one only for testing
void print_full_fs(void);

#endif // __FILE_SYSTEM_H__
