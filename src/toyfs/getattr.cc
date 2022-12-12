/**
 * @file getattr.cc
 * @author Hans WAN (hanswan@tom.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "toy.hh"
#include <unistd.h>
#include <ctime>

namespace toy {
  int toyfs::getattr(std::string path, struct stat *toy_stat) {
    // If is root directory
    if (!path.compare("/")) {
      toy_stat->st_mode = S_IFDIR | TOY_DEFAULT_MOD;
      toy_stat->st_size = sblock->get_sblock().block_address + TOY_DATA_BLOCK_SIZE * sblock->get_sblock().block_usage;
      toy_stat->st_nlink = 2;
    	toy_stat->st_uid 	 = getuid();
    	toy_stat->st_gid 	 = getgid();
    	toy_stat->st_atime   = time(NULL);
    	toy_stat->st_mtime   = time(NULL);
    	toy_stat->st_blksize = fs_io->io_size();
      toy_stat->st_blocks = fs_io->disk_size() / fs_io->io_size();
      return 0;
    }

    // Get target
    auto target = root_dir->get_path(path);
    if (target.ino == 0) {
      return -ENOENT; // not exist
    }

    toy_stat->st_nlink = 1;
    toy_stat->st_uid 	 = getuid();
    toy_stat->st_gid 	 = getgid();
    toy_stat->st_atime   = time(NULL);
    toy_stat->st_mtime   = time(NULL);
    toy_stat->st_blksize = fs_io->io_size();

    // Open the inode
    auto inode = inode_mgr->get_inode(target.ino);

    // Write basic information
    if (target.ftype == FILE) {
      toy_stat->st_mode = S_IFREG | TOY_DEFAULT_MOD;
      toy_stat->st_size = inode->size;
    } else {
      toy_stat->st_mode = S_IFDIR | TOY_DEFAULT_MOD;
      toy_stat->st_size = inode->children * sizeof(struct toy_dentry);
    }

    delete inode;
    return 0;
  }
}
