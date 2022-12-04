/**
 * @file toyfs.cc
 * @author Hans WAN (hanswan@tom.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#define FUSE_USE_VERSION 26

#include <iostream>
#include <cstring>
#include <fuse.h>

#include "toy.hh"

/********************************************************************
 * GLOBAL VARIABLES
 ********************************************************************/

struct custom_options {
	char *device;
	int show_help;
} options;

#define OPTION(t, p) { t, offsetof(struct custom_options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--device=%s", device),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

/********************************************************************
 * FILESYSTEM OBJECT
 ********************************************************************/

toy::toyfs *fs;

/********************************************************************
 * HOOK FUNCTIONS
 ********************************************************************/

void* toyfs_init(struct fuse_conn_info * conn_info) {
  try { 
    fs = new toy::toyfs(options.device); 
  } catch (...) {
    std::cerr << "Error(s) occurred while mounting filesystem";
    fuse_exit(fuse_get_context()->fuse);
  }
  return nullptr;
}

void toyfs_destory(void *p) {
  delete fs;
}


static struct fuse_operations operations = {
	.init = toyfs_init,
	.destroy = toyfs_destory,
};


int main(int argc, char *argv[])
{
  std::cout << "ToyFS: A simple ext2-like filesystem \n"
            << "Lab 5 of OS lecture in HITSZ \n";

  // Load default value
  options.device = (char *) std::malloc(64);
  std::strcpy(options.device, "/home/hans/ddriver");
  // Initialise args
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
  if (fuse_opt_parse(&args, &options, option_spec, nullptr) == -1) {
    std::cout << "Error(s) occurred while parsing arguments\n";
    return -1;
  }

  if (options.show_help) {
    std::cout << "Usage: toyfs --device=[device path] [mount point]\n";
    fuse_opt_free_args(&args);
    return 1;
  }

  std::cout << "Device path: " << options.device << "\n";

  int ret = fuse_main(args.argc, args.argv, &operations, nullptr);
  fuse_opt_free_args(&args);

  std::free(options.device);
  return ret;
}
