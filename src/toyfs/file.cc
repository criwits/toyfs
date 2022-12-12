/**
 * @file file.cc
 * @author Hans WAN (hanswan@tom.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "toy.hh"
#include <cstring>

namespace toy {
  file::file(io *fs_io, inode *node, blocks *blkio, inodes *inodeio):
    fs_io(fs_io), node(node), blkio(blkio), inodeio(inodeio) {
    // Do nothing!
  }

  file::~file() {}

  void file::sync() {
    node->sync();
  }

  void file::truncate(off_t offset) {
    int new_blk_cnt = CEIL(offset, TOY_DATA_BLOCK_SIZE) / TOY_DATA_BLOCK_SIZE;
    int original_blk_cnt = node->index.size();

    Log("Truncate, %d => %d", original_blk_cnt, new_blk_cnt);
    // Adjust blocks
    for (int i = 0; i < (original_blk_cnt - new_blk_cnt); i++) {
      // Case 1: original block count > new, release
      blkio->dealloc_block(node->index[node->index.size()]);
      node->index.pop_back();
    }
    for (int i = 0; i < (new_blk_cnt - original_blk_cnt); i++) {
      // Case 2: new > original, alloc
      node->index.push_back(blkio->alloc_block());
    }


    // sync node
    node->size = offset;
    node->sync();
  }

  int file::read(uint8_t *buf, size_t size, off_t offset) {
    // Check file size
    if (node->size < offset) {
      return -ESPIPE;
    }

    uint32_t lower_blk = FLOOR(offset, TOY_DATA_BLOCK_SIZE) / TOY_DATA_BLOCK_SIZE;
    uint32_t upper_blk = CEIL(offset + size, TOY_DATA_BLOCK_SIZE) / TOY_DATA_BLOCK_SIZE;
    std::vector<uint32_t> used_blks;
    for (uint32_t i = lower_blk; i < upper_blk; i++) {
      used_blks.push_back(node->index[i]);
    }

    if (used_blks.size() == 0) {
      return 0; // ???
    }

    if (used_blks.size() == 1) {
      // used one block, should just copy from head to end
      fs_io->read(blkio->get_addr(used_blks[0]) + offset, buf, size);
      return size;
    } else {
      // used more than one block, should treat head and tail differently

      // head block
      fs_io->read(blkio->get_addr(used_blks[0]) + offset, buf, TOY_DATA_BLOCK_SIZE - offset);
      buf += TOY_DATA_BLOCK_SIZE - offset;

      for (size_t i = 1; i < used_blks.size() - 1; i++) {
        fs_io->read(blkio->get_addr(used_blks[i]), buf, TOY_DATA_BLOCK_SIZE);
        buf += TOY_DATA_BLOCK_SIZE;
      }

      // tail block
      fs_io->read(blkio->get_addr(used_blks[used_blks.size() - 1]), 
        buf, used_blks.size() * TOY_DATA_BLOCK_SIZE - offset - size);

      return size;
    }

    return 0;
  }

  int file::write(const uint8_t *buf, size_t size, off_t offset) {
    Log("Target inode size: %d, writing to offset %d, size: %d", node->index.size(), offset, size);
    if (TOY_DATA_BLOCK_SIZE * node->index.size() < offset + size) {
      truncate(offset + size);
      Log("Target inode size: %d, writing to offset %d, size: %d", node->index.size(), offset, size);
    }
    // Check file size
    if (node->size < offset) {
      return -ESPIPE;
    }

    uint32_t lower_blk = FLOOR(offset, TOY_DATA_BLOCK_SIZE) / TOY_DATA_BLOCK_SIZE;
    uint32_t upper_blk = CEIL(offset + size, TOY_DATA_BLOCK_SIZE) / TOY_DATA_BLOCK_SIZE;
    std::vector<uint32_t> used_blks;
    for (uint32_t i = lower_blk; i < upper_blk; i++) {
      used_blks.push_back(node->index[i]);
    }

    if (used_blks.size() == 0) {
      return 0; // ???
    }

    if (used_blks.size() == 1) {
      // used one block, should just copy from head to end
      fs_io->write(blkio->get_addr(used_blks[0]) + offset, buf, size);
      node->size = (node->size > (offset + size)) ? (node->size) : (offset + size);
      return size;
    } else {
      // used more than one block, should treat head and tail differently

      // head block
      fs_io->write(blkio->get_addr(used_blks[0]) + offset, buf, TOY_DATA_BLOCK_SIZE - offset);
      buf += TOY_DATA_BLOCK_SIZE - offset;

      for (uint32_t i = 1; i < used_blks.size() - 1; i++) {
        fs_io->write(blkio->get_addr(used_blks[i]), buf, TOY_DATA_BLOCK_SIZE);
        buf += TOY_DATA_BLOCK_SIZE;
      }

      // tail block
      fs_io->write(blkio->get_addr(used_blks[used_blks.size() - 1]), 
        buf, used_blks.size() * TOY_DATA_BLOCK_SIZE - offset - size);

      node->size = (node->size > (offset + size)) ? (node->size) : (offset + size);
      return size;
    }

    return 0;
  }

}
