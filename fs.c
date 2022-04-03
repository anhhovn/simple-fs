#include <stdio.h>
#include "disk.h"
#include "fs.h"
#define FAT_EOF -1

typedef struct Superblock{
	int total_blocks;
	int root_index;
	int data_index;
}_super_blk;
