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
    if (sblock->check_magic()) {
      // Magic number detected
      sblock->print();

    } else {
      // Magic number invalid, init first
      Log("Warning: invalid magic number; initialise first");
      sblock->init();
    }

    // Mount and lock
    sblock->lock();
  }

  toyfs::~toyfs() {
    Log("ToyFS destructor, unmounting device");
    delete sblock;
    delete fs_io;
  }
};