/**
 * @file readdir.cc
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "toy.hh"

namespace toy {
  const std::vector<std::string> toyfs::readdir(std::string path) const {
    std::vector<std::string> ls;

    bool is_root = (path.compare("/") == 0);
    inode *node;
    directory *dir;

    if (is_root) {
      dir = root_dir;
    } else {
      auto dirino = root_dir->get_path(path);
      if (dirino.ino == 0 || dirino.ftype != DIR) {
        return ls;
      }

      node = inode_mgr->get_inode(dirino.ino);
      dir = new directory(fs_io, node, block_mgr, inode_mgr);
    }

    for (auto entry : dir->get_entries()) {
      ls.push_back(entry.fname);
    }

    if (!is_root) {
      delete dir;
      delete node;
    }

    return ls;
  }
}
