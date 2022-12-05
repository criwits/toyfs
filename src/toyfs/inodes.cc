/**
 * @file inodes.cc
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "toy.hh"

namespace toy {
  inodes::inodes(io *fs, superblock *super): fs_io(fs), sblock(super) {
    // Load bitmaps
    inode_bitmap = new bitmap(fs_io, sblock->get_sblock().inode_bitmap_address, sblock->get_sblock().inode_cnt);
    // Get args
    inode_addr = sblock->get_sblock().inode_address;
    inode_size = sizeof(struct toy_inode);
  }

  inodes::~inodes() {
    delete inode_bitmap;
  }

  uint32_t inodes::get_addr(uint32_t ino) {
    return inode_addr + ino * inode_size;
  }

  /**
   * @brief Create root inode and return it
   * 
   * @return inode* 
   */
  inode *inodes::create_root() {
    // In ext2 inode 0 and 1 are inaccessible, just occupy them
    inode_bitmap->set(0, 1);
    inode_bitmap->set(1, 1);
    sblock->inode_modify(2);

    auto root = new inode(fs_io, get_addr(2), 2, DIR);
    sblock->inode_modify(1);

    root->sync();
    return root;
  }
  
}
