/**
 * @file utils.cc
 * @author Hans WAN (hanswan@tom.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <cstring>
#include <cstdlib>
#include "toy.hh"
#include "ddriver.h"

namespace toy {
  io::io(std::string device): device_name(device) {
    // Open the device
    driver_fd = ddriver_open((char *) device.c_str());
    if (driver_fd < 0) {
      throw "Cannot open the ddriver file";
    }

    // Load driver size information
    ddriver_ioctl(driver_fd, IOC_REQ_DEVICE_SIZE, (&disk_sz));
    ddriver_ioctl(driver_fd, IOC_REQ_DEVICE_IO_SZ, (&io_sz));

    Log("ToyFS IO helper, disk size = %d, io size = %d", disk_sz, io_sz);
  }

  io::~io() {
    ddriver_close(driver_fd);
    Log("ToyFS IO helper, device detached");
  }

  /**
   * @brief Read bytes from device
   * 
   * @param addr The address (byte) to read
   * @param dest The destination to be written, must bigger than size
   * @param size The size
   */
  void io::read(uint32_t addr, uint8_t *dest, size_t size) const {
    Log("Reading from 0x%08x, size = 0x%08x", addr, size);
    // Some basic calculations
    uint32_t lower_bound = FLOOR(addr, io_sz);
    uint32_t upper_bound = CEIL(addr + size, io_sz);
    uint32_t read_blocks = (upper_bound - lower_bound) / io_sz;

    // Move to the location
    ddriver_seek(this->driver_fd, lower_bound, SEEK_SET);
    
    // Create a buffer
    uint8_t *buffer = (uint8_t *) std::malloc((size_t) read_blocks * io_sz);

    // Read block by block
    for (uint32_t i = 0; i < read_blocks; i++) {
      ddriver_read(driver_fd, (char *) (buffer + i * io_sz), io_sz);
    }

    // Copy to dest
    std::memcpy(dest, buffer + (addr - lower_bound), size);

    // Free the buffer
    std::free(buffer);
  }

  /**
   * @brief Write bytes to device
   * 
   * @param addr The address (byte) to be written
   * @param src The source
   * @param size The size
   */
  void io::write(uint32_t addr, const uint8_t *src, size_t size) const {
    Log("Write to 0x%08x, size = 0x%08x", addr, size);
    // Some basic calculations
    uint32_t lower_bound = FLOOR(addr, io_sz);
    uint32_t upper_bound = CEIL(addr + size, io_sz);
    uint32_t write_blocks = (upper_bound - lower_bound) / io_sz;

    // First read all contents
    uint8_t *buffer = (uint8_t *) std::malloc(write_blocks * io_sz);
    read(lower_bound, buffer, write_blocks * io_sz);

    // Alter some bytes
    std::memcpy(buffer + (addr - lower_bound), src, size);

    // Then write buffer back
    ddriver_seek(driver_fd, lower_bound, SEEK_SET);
    for (uint32_t i = 0; i < write_blocks; i++) {
      ddriver_write(driver_fd, (char *) (buffer + i * io_sz), io_sz);
    }

    // Free the buffer
    std::free(buffer);
  }

  /**
   * @brief Get the disk size
   * 
   * @return int 
   */
  const int io::disk_size() const {
    return disk_sz;
  }

  /**
   * @brief Get the IO size
   * 
   * @return const int 
   */
  const int io::io_size() const {
    return io_sz;
  }

}