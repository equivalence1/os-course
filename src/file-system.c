#include "file-system.h"

static inode_t  root;
struct spinlock global_fs_lock;

// TODO need locks!!!

void setup_fs(void) {
    root.parent = &root;
    root.file_name = (char *)"/";
    root.data = NULL;
    root.is_dir = true;
    list_init(&root.dir_files);

    spinlock_init(&global_fs_lock);
}

/**
 * if file with name `file_name` is in `dir` folder
 * then this method return pointer to it's inode.
 * if file_name == "", then it will return `dir`
 * NULL otherwise
 */
static inode_t* fs_dir_get_file(const inode_t *dir, const char *file_name) {
    if (strlen(file_name) == 0)
        return (inode_t *)dir;

    const struct list_head *dir_files = &dir->dir_files;
    int files_count = list_size(dir_files);

    for (int i = 0; i < files_count; i++) {
        dir_files = dir_files->next;
        inode_t *cur_file = ((inode_holder_t *)LIST_ENTRY(dir_files, inode_holder_t, holders))->child;
        if (strcmp(cur_file->file_name, file_name) == 0) {
            return cur_file;
        }
    }

    return NULL;
}

/**
 * parses path to some file and gets next folder from it.
 * i.e. for dir1/dir2/file.txt it will return `dir1`
 * for /dir1/dir2 it will return ""
 */
static bool get_next_dir_token(char *dist, const char *path) {
    uint64_t i = 0;
    while (path[i] != '/' && path[i] != '\0' && i < MAX_DIR_LEN) {
        dist[i] = path[i];
        i++;
    }
    dist[i] = '\0';

    return (path[i] == '/');
}

/**
 * dir -- dir which will contain this file
 * if `is_dir == true` then this file will be a directory
 */
static inode_t* create_file(inode_t *dir, const char *file_name, bool is_dir) {
    inode_t *file = (inode_t *)kmem_alloc(sizeof(inode_t));

    file->parent = dir;
    file->file_name = (char *)kmem_alloc(strlen(file_name) + 1);
    memcpy(file->file_name, file_name, strlen(file_name) + 1);
    file->data = NULL;
    file->size = 0;
    file->is_dir = is_dir;
    list_init(&file->dir_files);

    inode_holder_t *holder = (inode_holder_t *)kmem_alloc(sizeof(inode_holder_t));
    holder->child = file;
    list_init(&holder->holders);

    list_add_tail(&holder->holders, &dir->dir_files);

    return file;
}

/**
 * if `next_dir` is inside of `cur_dir` it will return
 * file descriptor for `next_dir`
 */
static inode_t* fs_cd(const inode_t *cur_dir, const char *next_dir) {
    inode_t *dir = fs_dir_get_file(cur_dir, next_dir);

    if (dir == NULL || dir->is_dir == false)
        return NULL;
    else
        return dir;
}

/**
 * creates a file which specified in `file_path`.
 * if should_create == false then it wont create a file
 * if some directory in this path does not exist.
 * if should_create == true it will create all intermediate directories.
 * if is_dir == true then this file should be created as dir
 */
static inode_t* fs_touch(const char *file_path, bool is_dir, bool should_create) {
    inode_t *cur_dir = &root;
    char *next_dir_token = (char *)kmem_alloc(MAX_DIR_LEN + 1);

    while (1) {
        if (!get_next_dir_token(next_dir_token, file_path))
            break;
        inode_t *next_dir = fs_cd(cur_dir, next_dir_token);
        if (next_dir == NULL) {
            if (should_create)
                next_dir = create_file(cur_dir, next_dir_token, true);
            else {
                kmem_free(next_dir_token);
                return NULL;
            }
        }
        cur_dir = next_dir;
        file_path += strlen(next_dir_token) + 1;
    }

    kmem_free(next_dir_token);

    inode_t *file = fs_dir_get_file(cur_dir, file_path);
    if (file == NULL)
        return create_file(cur_dir, file_path, is_dir);
    else
        return file;
}

fd_t *fs_open(const char *file_path, int flags) {
    spin_lock(&global_fs_lock);
    inode_t *node = fs_touch(file_path, false, flags & FLAG_CREATE);
    spin_unlock(&global_fs_lock);

    if (node == NULL) {
        printf("could not open '%s'. use FLAG_CREATE option to create file\n", file_path);
        return NULL;
    }

    fd_t *fd = (fd_t *)kmem_alloc(sizeof(fd_t));
    fd->file = node;
    spinlock_init(&fd->file_lock);
    fd->flags = flags;

    return fd;
}

void fs_close(fd_t *fd) {
    kmem_free(fd);
}

static void resize_file(inode_t *file, size_t new_size) {
    void *prev_data = file->data;
    file->data = kmem_alloc(new_size);
    if (prev_data != NULL) {
        memcpy(file->data, prev_data, file->size);
        kmem_free(prev_data);
    }
    file->size = new_size;
}

void fs_write(fd_t *fd, const void *buff, size_t size) {
    spin_lock(&fd->file_lock);
    if (!(fd->flags & FLAG_WRITE)) {
        printf("you can not write to file '%s'. Use option FLAG_WRITE\n", fd->file->file_name);
        spin_unlock(&fd->file_lock);
        return;
    }

    inode_t *file = fd->file;
    if (file->size < size)
        resize_file(file, size);

    for (size_t i = 0; i < size; i++) {
        *((char *)file->data + i) = ((char *)buff)[i];
    }

    spin_unlock(&fd->file_lock);
}

void fs_read(fd_t *fd, void *buff, size_t size) {
    spin_lock(&fd->file_lock);
    if (!(fd->flags & FLAG_READ)) {
        printf("you can not read from file '%s'. Use option FLAG_READ\n", fd->file->file_name);
        spin_unlock(&fd->file_lock);
        return;
    }

    inode_t *file = fd->file;

    for (size_t i = 0; i < MIN_CONST(file->size, size); i++) {
        ((char *)buff)[i] = *((char *)file->data + i);
    }

    spin_unlock(&fd->file_lock);
}

void fs_mkdir(const char *path) {
    spin_lock(&global_fs_lock);
    (void)fs_touch(path, true, true);
    spin_unlock(&global_fs_lock);
}

struct list_head* fs_readdir(const char *path) {
    spin_lock(&global_fs_lock);
    inode_t *cur_dir = &root;
    char *next_dir_token = (char *)kmem_alloc(MAX_DIR_LEN + 1);

    while (1) {
        if (!get_next_dir_token(next_dir_token, path))
            break;
        inode_t *next_dir = fs_cd(cur_dir, next_dir_token);
        if (next_dir == NULL) {
            kmem_free(next_dir_token);
            spin_unlock(&global_fs_lock);
            return NULL;
        }
        cur_dir = next_dir;
        path += strlen(next_dir_token) + 1;
    }

    kmem_free(next_dir_token);

    inode_t *file = fs_dir_get_file(cur_dir, path);

    spin_unlock(&global_fs_lock);
    if (file->is_dir == false)
        return NULL;
    else
        return &file->dir_files;
}

/**
 * I made this function only for testing purposes.
 */
static void print_dir(size_t indent, inode_t *file) {
    for (size_t i = 0; i < indent; i++)
        printf(" ");
    if (file->is_dir) {
        if (file != &root)
            printf("%s/\n", file->file_name);
        else
            printf("%s\n", file->file_name);
        struct list_head *dirs = &file->dir_files;
        if (strcmp(file->file_name, "dir2") == 0) // I do this __ONLY__ to test fs_readdir
            dirs = fs_readdir("/dir1/dir2");
        int size = list_size(dirs);
        for (int i = 0; i < size; i++) {
            dirs = dirs->next;
            print_dir(indent + 4, ((inode_holder_t *)LIST_ENTRY(dirs, inode_holder_t, holders))->child);
        }
    } else {
        printf("%s *\n", file->file_name);
    }
}

/**
 * I made this function only for testing purposes.
 */
void print_full_fs(void) {
    print_dir(0, &root);
}
