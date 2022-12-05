/**
 * @file blocks.cc
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
  blocks::blocks(io *fs, superblock *super): fs_io(fs), sblock(super) {
    // Load bitmaps
    block_bitmap = new bitmap(fs_io, sblock->get_sblock().block_bitmap_address, sblock->get_sblock().block_cnt);
    // Get args
    block_addr = sblock->get_sblock().block_address;
    block_size = TOY_DATA_BLOCK_SIZE;
  }

  blocks::~blocks() {
    delete block_bitmap;
  }


  uint32_t blocks::get_addr(uint32_t block_no) {
    return block_addr + block_no * block_size;
  }

  uint32_t blocks::create_root() {
    // block 0 is never used; just occupy
    block_bitmap->set(0, 1);
    sblock->block_modify(1);

    block_bitmap->set(1, 1);
    return 1;
  }

  uint32_t blocks::alloc_block() {
    // Check bitmap, if 0 then no valid block
    uint32_t val = block_bitmap->seek();
    if (val != 0) { 
      block_bitmap->set(val, 1); 
    }
    return val;
  }
}
