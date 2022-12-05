/**
 * @file directory.cc
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "toy.hh"
#include <cstring>

namespace toy {
  /**
   * @brief Load an inode as a directory
   * 
   * @param fs_io 
   * @param node 
   */
  directory::directory(io *fs_io, inode *node, blocks *blkio, inodes *inodeio): 
    fs_io(fs_io), node(node), blkio(blkio), inodeio(inodeio) {
    // Load indexes
    int children_cnt = 0;
    for (auto idx : node->index) {
      // idx now is a block number, first need to open it
      auto buf = (struct toy_dentry *) std::malloc(TOY_DATA_BLOCK_SIZE);
      fs_io->read(blkio->get_addr(idx), (uint8_t *) buf, TOY_DATA_BLOCK_SIZE);
      for (int i = 0; i < (TOY_DATA_BLOCK_SIZE / sizeof(struct toy_dentry)) && children_cnt < node->children; i++) {
        entries.push_back(buf[i]);
        children_cnt++;
      }
      std::free(buf);
    }
    Log("Loaded directory (inode %d): %d entries loaded", node->get_ino(), children_cnt);

  }

  directory::~directory() {
    sync();
  }
  
  /**
   * @brief Sync directory entries into disk -- 
   * 
   *  - update inode indexes (if necessary)
   *  - update dentries in data block
   * 
   */
  void directory::sync() {
    // First, calculate how many data blocks are needed
    auto entry_size = entries.size() * sizeof(struct toy_dentry);
    auto dblock_cnt = CEIL(entries.size(), (TOY_DATA_BLOCK_SIZE / sizeof(struct toy_dentry))) / (TOY_DATA_BLOCK_SIZE / sizeof(struct toy_dentry));

    Log("Current blocks: %d, need: %d", node->index.size(), dblock_cnt);

    // Adjust blocks if necessary
    for (int i = 0; i < (dblock_cnt - node->index.size()); i++) {
      node->index.push_back(blkio->alloc_block());
    }
    for (int i = 0; i < (node->index.size() - dblock_cnt); i++) {
      blkio->dealloc_block(node->index[node->index.size()]);
      node->index.pop_back();
    }

    // Write them back
    int children_cnt = 0;
    for (auto idx: node->index) {
      auto buf = (struct toy_dentry *) std::malloc(sizeof(struct toy_dentry));
      for (int i = 0; i < (TOY_DATA_BLOCK_SIZE / sizeof(struct toy_dentry)) && children_cnt < entries.size(); i++) {
        buf[i] = entries[children_cnt];
        children_cnt++;
      }
      fs_io->write(blkio->get_addr(idx), (uint8_t *) buf, TOY_DATA_BLOCK_SIZE);
      std::free(buf);
    }

    Log("Written back to directory (inode %d): %d entries in total", node->get_ino(), children_cnt);

    // Update some basic information
    node->children = entries.size();
    node->sync();

  }

  uint32_t directory::mkdir(std::string fname) {
    // First check if name exists
    for (auto entry : entries) {
      if (!fname.compare(entry.fname)) {
        Log("Warning: mkdir() with same filename");
        return 0;
      }
    }
    // Allocate inode
    auto ino = inodeio->alloc_inode(DIR);
    struct toy_dentry a = {
      .fname = "",
      .ino = ino,
      .ftype = DIR,
    };

    // Copy filename
    std::strncpy(a.fname, fname.c_str(), TOY_MAX_FILENAME_LENGTH);

    // Log into entries
    entries.push_back(a);
    sync();

    // Allocate block
    auto inode = inodeio->get_inode(ino);
    inode->index.push_back(blkio->alloc_block());
    inode->sync();

    delete inode;

    return ino;
  }

}
