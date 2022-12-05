/**
 * @file inode.cc
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
  /**
   * @brief Create a brand new inode, with given type and 0 size
   * 
   * @param fs 
   * @param address 
   * @param inode_number 
   * @param ftype 
   */
  inode::inode(io *fs, uint32_t address, uint32_t inode_number, file_type_t ftype): 
    fs_io(fs), addr(address), ino(inode_number), type(ftype) {
    size = 0;
    children = 0;
  }

  /**
   * @brief Construct an existing inode
   * 
   * @param fs 
   * @param address 
   */
  inode::inode(io *fs, uint32_t address): fs_io(fs), addr(address) {
    // Allocate a buffer
    struct toy_inode *buffer = (struct toy_inode *) std::malloc(sizeof(struct toy_inode));

    // Read from disk
    fs_io->read(addr, (uint8_t *) buffer, sizeof(struct toy_inode));
    
    // Copy information
    ino = buffer->ino;
    type = buffer->type;
    size = buffer->sz;
    children = buffer->dir_child_cnt;

    // Copy indexes
    for (int i = 0; i < TOY_INODE_DPTR; i++) {
      if (buffer->blk_dptr[i] != 0) {
        index.push_back(buffer->blk_dptr[i]);
      }
    }
    // TODO: INDIRECT INDEX

    // Delete the buffer
    std::free(buffer);
  }

  const uint32_t inode::get_ino() const {
    return ino;
  }

  inode::~inode() {}
  
  /**
   * @brief Sync inode into disk
   * 
   */
  void inode::sync() {
    // Allocate a buffer
    struct toy_inode *buffer = (struct toy_inode *) std::malloc(sizeof(struct toy_inode));

    // Write information
    buffer->ino = ino;
    buffer->type = type;
    buffer->sz = size;
    buffer->dir_child_cnt = children;
    
    for (int i = 0; i < TOY_INODE_DPTR; i++) {
      buffer->blk_dptr[i] = 0;
    }

    int i = 0;
    for (auto blkno : index) {
      buffer->blk_dptr[i++] = blkno;
    }

    // Write back
    fs_io->write(addr, (uint8_t *) buffer, sizeof(struct toy_inode));

    // Delete the buffer
    std::free(buffer);
  }
}