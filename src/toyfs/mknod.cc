/**
 * @file mknod.cc
 * @author Hans WAN (hanswan@tom.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "toy.hh"

namespace toy {
  /**
   * @brief Create directory
   * 
   * @param path 
   * @param mode 
   * @return int 
   */
  int toyfs::mknod(std::string path) {
    bool is_root_dir = (path.find_last_of('/') == 0);
    directory *parent_dir;
    inode *parent_inode;
    // if root
    if (is_root_dir) {
      parent_dir = root_dir;
    } else {
      // First, get the parent
      Log("Trying to mknod %s", path.c_str());
      auto parent = root_dir->get_path(path.substr(0, path.find_last_of('/')));
      if (parent.ino == 0 || parent.ftype != DIR) {
        return ENOENT;
      }

      // Open the directory
      parent_inode = inode_mgr->get_inode(parent.ino);
      parent_dir = new directory(fs_io, parent_inode, block_mgr, inode_mgr);
    }


    // Create dir
    int ret;
    if (parent_dir->mknod(path.substr(path.find_last_of('/') + 1)) == 0) {
      ret = -EEXIST;
    } else {
      ret = 0;
    }

    if (!is_root_dir) {
      delete parent_dir;
      delete parent_inode;
    }

    return ret;
  }
}