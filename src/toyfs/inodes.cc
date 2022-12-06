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

    inode_bitmap->set(2, 1);
    auto root = new inode(fs_io, get_addr(2), 2, DIR);
    sblock->inode_modify(1);

    root->sync();
    return root;
  }

  /**
   * @brief Get an inode with given ino
   * 
   * @param ino 
   * @return inode* 
   */
  inode *inodes::get_inode(uint32_t ino) {
    return new inode(fs_io, get_addr(ino));
  }

  uint32_t inodes::alloc_inode(file_type_t ftype) {
    // Check bitmap
    uint32_t val = inode_bitmap->seek();
    if (val == 0) {
      return 0;
    }
    inode_bitmap->set(val, 1);
    sblock->inode_modify(1);

    // Create basic structure
    auto node = new inode(fs_io, get_addr(val), val, ftype);
    node->sync();
    delete node;
    return val;
  }

  void inodes::dealloc_inode(uint32_t ino) {
    // Check bitmap, if 0 then return
    if (inode_bitmap->get(ino) == 0) {
      return;
    }
    inode_bitmap->set(ino, 0);
    sblock->inode_modify(-1);

  }

  void inodes::sync() {
    inode_bitmap->sync();
  }

  
}
