/**
 * @file file.cc
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
  file::file(io *fs_io, inode *node, blocks *blkio, inodes *inodeio):
    fs_io(fs_io), node(node), blkio(blkio), inodeio(inodeio) {
    // Do nothing!
  }

  file::~file() {}

  void file::sync() {
    node->sync();
  }
}
