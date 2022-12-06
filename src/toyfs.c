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

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "toyfs.h"


/********************************************************************
 * GLOBAL VARIABLES
 ********************************************************************/

struct custom_options options;

#define OPTION(t, p) { t, offsetof(struct custom_options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--device=%s", device),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};


static struct fuse_operations operations = {
	.init = toyfs_init,
	.destroy = toyfs_destory,
  .getattr = toyfs_getattr,
  .mkdir = toyfs_mkdir,
  .readdir = toyfs_readdir,
  .mknod = toyfs_mknod,
  .utimens = toyfs_utimens
};


int main(int argc, char *argv[])
{
  printf("ToyFS: 0.0.1-SNAPSHOT\n");

  // Load default value
  options.device = (char *) malloc(64);
  strcpy(options.device, "/home/hans/ddriver");
  // Initialise args
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
  if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1) {
    printf("Error(s) occurred while parsing arguments\n");
    return -1;
  }

  if (options.show_help) {
    printf("Usage: toyfs --device=[device path] [mount point]\n");
    fuse_opt_free_args(&args);
    return 1;
  }


  int ret = fuse_main(args.argc, args.argv, &operations, NULL);
  fuse_opt_free_args(&args);

  free(options.device);
  return ret;
}
