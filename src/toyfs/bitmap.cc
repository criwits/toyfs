/**
 * @file bitmap.cc
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "toy.hh"
#include <cstdlib>

namespace toy {
  bitmap::bitmap(io *fs, uint32_t address, uint32_t cnt): fs_io(fs), addr(address), bitcnt(cnt) {
    // Calculate size
    size = CEIL(bitcnt, 8) / 8;
    // Create memory buffer to store bitmap
    buffer = (uint8_t *) std::malloc(size);
    // Load disk bitmap into memory
    fs_io->read(addr, buffer, size);
  }

  bitmap::~bitmap() {
    sync();
    std::free(buffer);
  }

  bool bitmap::get(uint32_t address) {
    uint32_t byte_no = address / 8;
    uint32_t byte_offset = address - byte_no * 8;
    return ((buffer[byte_no]) >> (7 - byte_offset)) & 0x1;
  }

  void bitmap::set(uint32_t address, bool bit) {
    uint32_t byte_no = address / 8;
    uint32_t byte_offset = address - byte_no * 8;

    uint8_t original = buffer[byte_no];
    uint8_t mask = ~(1 << (7 - byte_offset)); // something like 1101_1111
    uint8_t set_mask = (bit ? 1 : 0) << (7 - byte_offset); // something like 0010_0000

    buffer[byte_no] = (original & mask) | set_mask;
  }

  uint32_t bitmap::seek() {
    for (uint32_t i = 0; i < bitcnt; i++) {
      if (!get(i)) {
        return i;
      }
    }
    return 0; // should treat this
  }

  uint32_t bitmap::count() {
    uint32_t cnt = 0;
    for (uint32_t i = 0; i < bitcnt; i++) {
      if (!get(i)) {
        cnt++;
      }
    }
    return cnt;
  }

  void bitmap::sync() {
    fs_io->write(addr, buffer, size);
  }
}