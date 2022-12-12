/**
 * @file readwrite.cc
 * @author Hans WAN (hanswan@tom.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "toy.hh"

namespace toy {
  int toyfs::read(std::string path, uint8_t *buf, size_t size, off_t offset) {
    bool is_root_dir = (path.find_last_of('/') == 0);
    directory *parent_dir;
    inode *parent_inode;
    // if root
    if (is_root_dir) {
      parent_dir = root_dir;
    } else {
      // First, get the parent
      auto parent = root_dir->get_path(path.substr(0, path.find_last_of('/')));
      if (parent.ino == 0 || parent.ftype != DIR) {
        return -ENOENT;
      }

      // Open the directory
      parent_inode = inode_mgr->get_inode(parent.ino);
      parent_dir = new directory(fs_io, parent_inode, block_mgr, inode_mgr);
    }


    // Get the file
    int ret;
    auto fino = parent_dir->get_child(path.substr(path.find_last_of('/') + 1));
    if (fino.ino == 0) { 
      ret = -ENOENT; 
    } else
    if (fino.ftype == DIR) {
      ret = -EISDIR;
    } else {
      auto finode = inode_mgr->get_inode(fino.ino);
      auto f = new file(fs_io, finode, block_mgr, inode_mgr);

      ret = f->read(buf, size, offset);

      delete f;
      delete finode;
    }

    if (!is_root_dir) {
      delete parent_dir;
      delete parent_inode;
    }

    return ret;
  }

  int toyfs::write(std::string path, const uint8_t *buf, size_t size, off_t offset) {
    bool is_root_dir = (path.find_last_of('/') == 0);
    directory *parent_dir;
    inode *parent_inode;
    // if root
    if (is_root_dir) {
      parent_dir = root_dir;
    } else {
      // First, get the parent
      auto parent = root_dir->get_path(path.substr(0, path.find_last_of('/')));
      if (parent.ino == 0 || parent.ftype != DIR) {
        return -ENOENT;
      }

      // Open the directory
      parent_inode = inode_mgr->get_inode(parent.ino);
      parent_dir = new directory(fs_io, parent_inode, block_mgr, inode_mgr);
    }


    // Get the file
    int ret;
    auto fino = parent_dir->get_child(path.substr(path.find_last_of('/') + 1));
    if (fino.ino == 0) { 
      ret = -ENOENT; 
    } else
    if (fino.ftype == DIR) {
      ret = -EISDIR;
    } else {
      auto finode = inode_mgr->get_inode(fino.ino);
      auto f = new file(fs_io, finode, block_mgr, inode_mgr);

      Log("Trying to write into file with inode %d", fino.ino);
      ret = f->write(buf, size, offset);

      f->sync();
      delete f;
      finode->sync();
      delete finode;
    }

    if (!is_root_dir) {
      delete parent_dir;
      delete parent_inode;
    }

    return ret;
  }
}
