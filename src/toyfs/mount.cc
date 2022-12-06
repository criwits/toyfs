/**
 * @file toy.cc
 * @author Hans WAN (hanswan@tom.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "toy.hh"
#include "ddriver.h"
#include "consts.hh"
#include <iostream>
#include <string>

namespace toy {
  /**
   * @brief Mount a toyfs partition, construct a new toyfs object
   * 
   * @param device Path of device to mount
   */
  toyfs::toyfs(std::string device) {
    Log("ToyFS initialiser, current device path = %s", device.c_str());

    // Initialise IO helper
    fs_io = new io(device);

    // Read superblock
    sblock = new superblock(fs_io);

    // Check magic number
    bool is_init = false;
    if (sblock->check_magic()) {
      // Magic number detected
      sblock->print();
    } else {
      // Magic number invalid, init first
      Log("Warning: invalid magic number; initialise first");
      sblock->init();
      sblock->sync();
      is_init = true;
    }

    // Change sblock status
    sblock->lock();

    // Initialise inode manager
    inode_mgr = new inodes(fs_io, sblock);
    // Initialise block manager
    block_mgr = new blocks(fs_io, sblock);

    if (is_init) {
      // Create root inode, empty
      auto root = inode_mgr->create_root();
      // Allocate block for root dentry

      auto block_no = block_mgr->create_root();
      root->index.push_back(block_no);
      root->sync();

      delete root;
    }

    root_inode = inode_mgr->get_inode(2);

    // Open root directory
    root_dir = new directory(fs_io, root_inode, block_mgr, inode_mgr);
  }

  toyfs::~toyfs() {

    Log("Syncing root directory");
    root_dir->sync();
    delete root_dir;

    Log("Syncing root inode");
    root_inode->sync();
    delete root_inode;

    Log("Syncing bitmaps");
    inode_mgr->sync();
    block_mgr->sync();
    delete inode_mgr;
    delete block_mgr;

    Log("Syncing superblock");
    sblock->unlock();
    sblock->sync();
    delete sblock;

    Log("Detaching IO helper");
    delete fs_io;
  }
}