/**
 * @file sblock.cc
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "toy.hh"
#include "consts.hh"


namespace toy {
  superblock::superblock(io *fs_io) : fs_io(fs_io) {
    // Load super block from the disk
    fs_io->read(TOY_SBLOCK_OFFSET, (uint8_t *) (&sblock), sizeof(sblock));
  }

  superblock::~superblock() {}

  void superblock::lock() {
    sblock.status = MOUNTED;
  }

  void superblock::unlock() {
    sblock.status = UNMOUNTED;
  }

  bool superblock::check_magic() {
    return sblock.magic == TOY_MAGIC_NUMBER;
  }

  void superblock::init() {
    // Do some mathematical problems
    uint32_t disk_size = fs_io->disk_size();
    uint32_t io_size = fs_io->io_size();
    uint32_t inode_size = sizeof(struct toy_inode);

    uint32_t sblock_size = CEIL(sizeof(struct toy_superblock), io_size);
    uint32_t inode_estimate_cnt = disk_size / TOY_ESTIMATE_FILESIZE;
    uint32_t inode_bitmap_size = CEIL(inode_estimate_cnt / 8, io_size);
    uint32_t inode_cnt = inode_bitmap_size * 8;
    uint32_t inode_total_size = CEIL(inode_size * inode_cnt, io_size);

    uint32_t block_estimate_cnt = 8 * (disk_size - sblock_size - inode_bitmap_size - inode_total_size) / (1 + 8 * TOY_DATA_BLOCK_SIZE);
    uint32_t block_bitmap_size = CEIL(block_estimate_cnt / 8, io_size);
    uint32_t block_cnt = (disk_size - sblock_size - inode_bitmap_size - inode_total_size - block_bitmap_size) / TOY_DATA_BLOCK_SIZE;

    Log("Estimated filesystem structure (in byte):");
    Log("\tSuperblock   [0x%08x, 0x%08x)", TOY_SBLOCK_OFFSET, sblock_size);
    Log("\tInode Bitmap [0x%08x, 0x%08x) => %d bits for %d inodes", TOY_SBLOCK_OFFSET + sblock_size, 
      TOY_SBLOCK_OFFSET + sblock_size + inode_bitmap_size, 
      inode_bitmap_size * 8, 
      inode_cnt);
    Log("\tBlock Bitmap [0x%08x, 0x%08x) => %d bits for %d blocks", TOY_SBLOCK_OFFSET + sblock_size + inode_bitmap_size, 
      TOY_SBLOCK_OFFSET + sblock_size + inode_bitmap_size + block_bitmap_size, 
      block_bitmap_size * 8, 
      block_cnt);
    Log("\tInodes       [0x%08x, 0x%08x) => %d items for %d inodes", TOY_SBLOCK_OFFSET + sblock_size + inode_bitmap_size + block_bitmap_size, 
      TOY_SBLOCK_OFFSET + sblock_size + inode_bitmap_size + block_bitmap_size + inode_total_size,
      inode_total_size / inode_size, 
      inode_cnt);
    Log("\tData         [0x%08x, 0x%08x) => %d blocks", TOY_SBLOCK_OFFSET + sblock_size + inode_bitmap_size + block_bitmap_size + inode_total_size, 
      TOY_SBLOCK_OFFSET + sblock_size + inode_bitmap_size + block_bitmap_size + inode_total_size + block_cnt * TOY_DATA_BLOCK_SIZE, block_cnt);
    
    sblock.magic = TOY_MAGIC_NUMBER;
    sblock.status = UNMOUNTED;
    
    sblock.inode_cnt = inode_cnt;
    sblock.inode_usage = 0;
    sblock.block_cnt = block_cnt;
    sblock.block_usage = 0;

    sblock.inode_bitmap_address = TOY_SBLOCK_OFFSET + sblock_size;
    sblock.block_bitmap_address = TOY_SBLOCK_OFFSET + sblock_size + inode_bitmap_size;
    sblock.inode_address = TOY_SBLOCK_OFFSET + sblock_size + inode_bitmap_size + block_bitmap_size;
    sblock.block_address = TOY_SBLOCK_OFFSET + sblock_size + inode_bitmap_size + block_bitmap_size + inode_total_size;
  }

  const struct toy_superblock superblock::get_sblock() const {
    return sblock;
  }

  void superblock::print() {
    Log("Superblock information");
    Log("\tMagic number = 0x%08x", sblock.magic);
    Log("\tInodes: %8d of %8d used (%.2f%%)", sblock.inode_usage, sblock.inode_cnt, sblock.inode_usage / (float) sblock.inode_cnt);
    Log("\tBlocks: %8d of %8d used (%.2f%%)", sblock.block_usage, sblock.block_cnt,sblock.block_usage / (float) sblock.block_cnt);
  }

  void superblock::inode_modify(int offset) {
    sblock.inode_usage += offset;
  }

  void superblock::block_modify(int offset) {
    sblock.block_usage += offset;
  }

  void superblock::sync() {
    fs_io->write(TOY_SBLOCK_OFFSET, (uint8_t *) (&sblock), sizeof(sblock));
  }

}