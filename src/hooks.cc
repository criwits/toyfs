/**
 * @file hooks.cc
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "toyfs.h"
#include "toy.hh"

#include <iostream>

extern struct custom_options options;

/********************************************************************
 * FILESYSTEM OBJECT
 ********************************************************************/

toy::toyfs *fs;

/********************************************************************
 * HOOK FUNCTIONS
 ********************************************************************/

void* toyfs_init(struct fuse_conn_info * conn_info) {
  try { 
    fs = new toy::toyfs(options.device); 
  } catch (...) {
    std::cerr << "Error(s) occurred while mounting filesystem";
    fuse_exit(fuse_get_context()->fuse);
  }
  return nullptr;
}

void toyfs_destory(void *p) {
  delete fs;
}

int toyfs_getattr(const char *path, struct stat *toy_stat) {
  return fs->getattr(path, toy_stat);
}

int toyfs_mkdir(const char *path, mode_t mode) {
  return fs->mkdir(path);
}

int toyfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
  auto ls = fs->readdir(path);

  if (ls.size() == 0) {
    return -ENOENT;
  }

  if ((uint32_t) offset < ls.size()) { // unsigned???
    filler(buf, ls[offset].c_str(), NULL, offset + 1);
  }

  return 0;
}

int toyfs_mknod(const char* path, mode_t mode, dev_t dev) {
  return fs->mknod(path);
}

int toyfs_utimens(const char* path, const struct timespec tv[2]) {
  return 0;
}


int toyfs_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info *fi) {
  return fs->read(path, (uint8_t *) buf, size, offset);
}

int toyfs_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info *fi) {
  return fs->write(path, (const uint8_t *) buf, size, offset);
}

int toyfs_truncate(const char *path, off_t offset) {
  return fs->truncate(path, offset);
}

int toyfs_open(const char* path, struct fuse_file_info* fi) {
  return 0;
}

int toyfs_opendir(const char* path, struct fuse_file_info* fi) {
  return 0;
}

int toyfs_access(const char* path, int type) {
  struct stat s;
  switch (type) {
    case R_OK:
    case W_OK:
    case X_OK:
      return 0;
    case F_OK:
      return toyfs_getattr(path, &s) == 0 ? 0 : -EACCES;
  }
  return 0; // shall never reach
}
