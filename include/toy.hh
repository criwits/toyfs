/**
 * @file toy.hh
 * @author Hans WAN (hanswan@tom.com)
 * @brief Header for ToyFS
 * @version 0.1
 * @date 2022-12-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _TOY_HH_
#define _TOY_HH_

#include <cstdint>
#include <string>
#include "consts.hh"
#include "log.hh"

#define FLOOR(value, align) (value % align == 0 ? value : (value / align) * align)
#define CEIL(value, align) (value % align == 0 ? value : (value / align + 1) * align)

namespace toy {
  class io {
    std::string device_name;
    int driver_fd;
    int disk_sz;
    int io_sz;
  public:
    io(std::string);
    ~io();
    void read(uint32_t, uint8_t *, size_t) const;
    void write(uint32_t, const uint8_t *, size_t) const;
    const int disk_size() const;
    const int io_size() const;
  };
  
  typedef enum {
    UNMOUNTED, MOUNTED
  } pstatus_t;

  struct toy_superblock {
      // Magic number
      uint32_t magic;

      // Partition status
      pstatus_t status;

      uint32_t inode_cnt;
      uint32_t inode_usage;
      uint32_t block_cnt;
      uint32_t block_usage;

      uint32_t inode_bitmap_address;
      uint32_t block_bitmap_address;
      uint32_t inode_address;
      uint32_t block_address;
  };

  typedef enum file_type {
    FILE, DIR
  } file_type_t;

  /**
   * @brief INode for ToyFS
   * 
   */
  struct toy_inode {
    // INode number
    uint32_t ino;

    // Corresponding file size (byte)
    uint32_t sz;
    // File type
    file_type_t type;
    // If it is directory, this field stores # of children; otherwise 0
    uint32_t dir_child_cnt;

    // Direct indexes; 0 for unused; shall be sorted in order
    uint32_t blk_dptr[TOY_INODE_DPTR];
    // Indirect indexes
    uint32_t blk_iptr[TOY_INODE_IPTR];
  };


  class superblock {
    io *fs_io;
    struct toy_superblock sblock;

  public:
    superblock(io *);
    ~superblock();  
    void init();
    void sync();
    void lock();
    void unlock();
    bool check_magic();
    const struct toy_superblock get_sblock() const;
    void print();
  };


  class toyfs {
    io *fs_io;
    superblock *sblock;

  public:
    toyfs(std::string);
    ~toyfs();
  };
};

#endif