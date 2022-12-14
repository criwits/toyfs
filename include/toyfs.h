/**
 * @file toyfs.h
 * @author Hans WAN (hanswan@tom.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _TOYFS_H_
#define _TOYFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fuse.h>
#include <sys/stat.h>

struct custom_options {
	char *device;
	int show_help;
};

void* toyfs_init(struct fuse_conn_info *conn_info);
void toyfs_destory(void *p);
int toyfs_getattr(const char *path, struct stat *toy_stat);
int toyfs_mkdir(const char *path, mode_t mode);
int toyfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int toyfs_mknod(const char* path, mode_t mode, dev_t dev);
int toyfs_utimens(const char* path, const struct timespec tv[2]);
int toyfs_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info *fi);
int toyfs_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info *fi);
int toyfs_truncate(const char *path, off_t offset);
int toyfs_open(const char* path, struct fuse_file_info* fi);
int toyfs_opendir(const char* path, struct fuse_file_info* fi);
int toyfs_access(const char* path, int type);

#ifdef __cplusplus
}
#endif

#endif