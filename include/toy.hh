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
#include <vector>
#include "consts.hh"
#include "log.hh"

#define FLOOR(value, align) ((value) % (align) == 0 ? (value) : ((value) / (align)) * (align))
#define CEIL(value, align) ((value) % (align) == 0 ? (value) : ((value) / (align) + 1) * (align))

namespace toy {
  /*******************************************************************
   * TYPES AND STRUCTS
  ********************************************************************/
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
    size_t sz;
    // File type
    file_type_t type;
    // If it is directory, this field stores # of children; otherwise 0
    uint32_t dir_child_cnt;

    // Direct indexes; 0 for unused; shall be sorted in order
    uint32_t blk_dptr[TOY_INODE_DPTR];
    // Indirect indexes
    // uint32_t blk_iptr[TOY_INODE_IPTR];
  };

  struct toy_dentry {
    char fname[256];
    uint32_t ino;
    file_type_t ftype;
  };

  /*******************************************************************
   * CLASSES
   ********************************************************************/

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
    void inode_modify(int);
    void block_modify(int);
  };

  class bitmap {
    io *fs_io;
    uint32_t addr;
    uint32_t bitcnt;

    size_t size;
    uint8_t *buffer;

  public:
    bitmap(io *, uint32_t, uint32_t);
    ~bitmap();
    bool get(uint32_t);
    void set(uint32_t, bool);
    uint32_t seek();
    uint32_t count();
    void sync();
  };

  class inode {
    io *fs_io;
    uint32_t addr;

    uint32_t ino;
    file_type_t type;


    inode(io *, uint32_t, uint32_t, file_type_t);
    inode(io *, uint32_t);
  public:
    uint32_t size;
    uint32_t children;
    std::vector<uint32_t> index;
    ~inode();
    void sync();
    friend class inodes;
  };

  class inodes {
    io *fs_io;
    superblock *sblock;
    bitmap *inode_bitmap;

    uint32_t inode_addr;
    size_t inode_size;
  public:
    inodes(io *, superblock *);
    ~inodes();
    uint32_t get_addr(uint32_t);
    inode *create_root();
    inode *alloc_inode();
  };

  class blocks {
    io *fs_io;
    superblock *sblock;
    bitmap *block_bitmap;

    uint32_t block_addr;
    size_t block_size;
  public:
    blocks(io *, superblock *);
    ~blocks();
    uint32_t get_addr(uint32_t);
    uint32_t create_root();
    uint32_t alloc_block();
  };

  class directory {
    io *fs_io;
  public:
    std::vector<struct toy_dentry> entries;
    directory(io *, inode *);
    ~directory();

  };






  class toyfs {
    io *fs_io;
    superblock *sblock;
    inodes *inode_mgr;
    blocks *block_mgr;
  public:
    toyfs(std::string);
    ~toyfs();
  };



}

#endif