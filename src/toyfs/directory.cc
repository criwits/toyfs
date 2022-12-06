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

  directory::~directory() {}
  
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
    if (dblock_cnt == 0) { 
      dblock_cnt = 1; 
    }

    // Adjust blocks if necessary
    for (int i = 0; i < (dblock_cnt - node->index.size()); i++) {
      // Case 1: dblock_cnt > index.size(), need allocate more blocks
      node->index.push_back(blkio->alloc_block());
    }
    for (int i = 0; i < (node->index.size() - dblock_cnt); i++) {
      // Case 2: index.size() > dblock_cnt, can reduce block usage
      blkio->dealloc_block(node->index[node->index.size()]);
      node->index.pop_back();
    }

    // Write them back
    int children_cnt = 0;
    for (auto idx: node->index) {
      // Allocate buffer for block
      auto buf = (struct toy_dentry *) std::malloc(TOY_DATA_BLOCK_SIZE);
      // Then write back entries
      for (int i = 0; i < (TOY_DATA_BLOCK_SIZE / sizeof(struct toy_dentry)) && children_cnt < entries.size(); i++) {
        buf[i] = entries[children_cnt];
        children_cnt++;
      }
      fs_io->write(blkio->get_addr(idx), (uint8_t *) buf, TOY_DATA_BLOCK_SIZE);
      std::free(buf);
    }

    // Update some basic information
    node->children = entries.size();
    node->sync();
  }

  /**
   * @brief Get child
   * 
   * @param fname 
   * @return struct toy_dentry 
   */
  struct toy_dentry directory::get_child(std::string fname) {
    for (auto entry : entries) {
      if (!fname.compare(entry.fname)) {
        Log("Get child %s: inode %d, %s", fname.c_str(), entry.ino, entry.ftype == FILE ? "file" : "directory");
        return entry;
      }
    }
    struct toy_dentry null = {
      .fname = "",
      .ino = 0,
      .ftype = FILE
    };
    Log("Get child %s: failed, doesn't exist", fname.c_str());
    return null;
  }

  struct toy_dentry directory::get_path(std::string path) {
    // Check if root
    if (path.compare("/") == 0) {
      struct toy_dentry root = {
        .fname = "",
        .ino = 2,
        .ftype = DIR
      };
      return root;
    }

    // Check if the last layer
    if (path.find_last_of('/') == 0) {
      return get_child(path.substr(1));
    }

    // Then check one by one
    auto child = get_child(path.substr(1, path.find_first_of('/', 1) - 1));
    if (child.ino == 0 || child.ftype == FILE) { // shall never be FILE
      // no such record
      struct toy_dentry null = {
        .fname = "",
        .ino = 0,
        .ftype = FILE
      };
      return null;
    }

    Log("Current child: %s", child.fname);

    // Open subdirectory
    /// open inode
    auto subinode = inodeio->get_inode(child.ino);
    /// open directory
    auto subdirectory = new directory(fs_io, subinode, blkio, inodeio);
    /// get result
    auto result = subdirectory->get_path(path.substr(path.find_first_of('/', 1)));
    delete subdirectory;
    delete subinode;
    return result;
  }


  /**
   * @brief Create a new directory under current dir
   * 
   * @param fname 
   * @return uint32_t ino; will be 0 if fails
   */
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

  uint32_t directory::mknod(std::string fname) {
    // First check if name exists
    for (auto entry : entries) {
      if (!fname.compare(entry.fname)) {
        Log("Warning: mknod() with same filename");
        return 0;
      }
    }
    // Allocate inode
    auto ino = inodeio->alloc_inode(FILE);
    struct toy_dentry a = {
      .fname = "",
      .ino = ino,
      .ftype = FILE,
    };

    // Copy filename
    std::strncpy(a.fname, fname.c_str(), TOY_MAX_FILENAME_LENGTH);

    // Log into entries
    entries.push_back(a);
    sync();

    return ino;
  }

  const std::vector<struct toy_dentry> directory::get_entries() const {
    return entries;
  }

}
