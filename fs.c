#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "disk.h"
#include "fs.h"

#define EMPTY 0
#define END_OF_FILE -1

/*
super_block:
This is the first block of the disk and it contains informataion about the location of the other
data structures (FAT, root directory, and the start of the data blocks).

num_blocks           - total of blocks on virtual disk
ind_root_dir         - index of root directory
ind_start_data_block - index of the first data block
ind_FAT              - index of FAT
num_FAT_blocks       - total number of FAT blocks
num_data_blocks      - total number of data blocks 
*/
struct super_block{
	int num_blocks;
	int ind_root_dir;
	int ind_start_data_block;
	int ind_FAT;
	int num_FAT_blocks;
	int num_data_blocks;
};


/*
File allocation table (FAT):

This is the block after superblock. FAT can take more than one block on the disk depends on the number
of data blocks. It is used to keep track of empty blocks and the mapping between files and their data blocks.

It presents an array of structs containing entry
ind_entry            

*/

struct FAT{
	int ind_entry;
};

/*
rootDirectory:

The block next to FAT block/blocks locates root directory.
It represents an array of structs which define, for each file, the file size and the head of the list of corresponding
data blocks.

fileName           - the name of the file
file_size          - the size of the file
first_data_block   - the location of the first data block for this file

*/
struct rootDirectory{
	char fileName[FILENAME_LEN_MAX];
	int file_size;
	int first_data_block;
	bool isActive;	
};

/*
fileDescriptor:
It represents an array of structs which define, for each file descriptor, an index in the range between 0 and 31 (inclusive)
A file descriptor contains a file offset, the name of the file it is associated with, and a boolean variable to update its 
status
*/
struct fileDescriptor{
	char fileName[FILENAME_LEN_MAX];
	bool isUsed;
	off_t offset;
	int ind;
};
/*
Initialize variables

*/
struct super_block    *superblock;
struct rootDirectory  *root_dir;
struct FAT            *fat;
struct fileDescriptor file_descriptors[FILE_OPEN_MAX];
char str[1000];

/*make_fs
num_blocks           - total of blocks on virtual disk
ind_root_dir         - index of root directory
ind_start_data_block - index of the first data block
ind_FAT              - index of FAT
num_FAT_entries      - total number of FAT entries
num_data_blocks      - total number of data blocks */ 
int make_fs( char *disk_name){
	 //create and open new disk
	 make_disk(disk_name);
	 open_disk(disk_name);

	 //initialize and write meta-information for file system


	 /*initialize superblock
	 Each FAT entry holds an integer (4 bytes) which defines the index of the other entry.
    4096 data blocks (4096 numbers of integer) * 4 = 16384 bytes is the size of the FAT table
    16384 / BLOCK_SIZE = 4 blocks needed for FAT table

	 */

	 superblock = malloc(BLOCK_SIZE);
	 if(superblock == NULL) return -1;
	 superblock -> num_blocks           = DISK_BLOCKS;
	 superblock -> ind_FAT              = 1;
	 superblock -> num_FAT_blocks       = 4;
	 superblock -> ind_root_dir         = 5;
	 superblock -> ind_start_data_block = 4096; // 4096 data blocks, index in the range between [4096, 8191]
	 superblock -> num_data_blocks      = 4096;

	 /*write superblock to disk*/
	 block_write(0, (void*)superblock);
	 free(superblock);
	 close_disk();
	 printf("make successfully\n");
    return 0;
 } 

/*mount_fs*/
int mount_fs(char *disk_name){
	//if(disk_name == NULL) return -1;
	open_disk(disk_name);
	//read super block
	superblock = malloc(BLOCK_SIZE);
	block_read(0, (void*)superblock);
  
   
  //initialize FAT blocks
  fat = malloc((superblock->num_FAT_blocks) * BLOCK_SIZE);
  for(int i = 0; i < superblock -> num_FAT_blocks; i ++){
   	block_read(i+1, (void*)fat + (i*BLOCK_SIZE)); // read four consecutive 4096-bytes blocks  
  }

	/*initialize directory information*/
  root_dir = malloc(BLOCK_SIZE);
  block_read((superblock->num_FAT_blocks) + 1, (void*)root_dir);

  for(int i = 0; i <FILE_NUM_MAX; i++){
  	root_dir[i].isActive = false;
  }


   /*get file descriptor ready*/
  for(int i = 0; i < FILE_OPEN_MAX ; i++){
   	file_descriptors[i].isUsed = false;
  }

  printf("//======mount_fs()======//\n");
  printf("%s mounted successfully\n",disk_name);
  printf("\n");

   
	return 0;
}

int umount_fs(char *disk_name){
	//if(disk_name == NULL) return -1;
   
   char buf[BLOCK_SIZE];
   /*write super block*/
   block_write(0,(void*)superblock);
   
	/*write FAT table*/
	for(int i = 0; i <= superblock-> num_FAT_blocks; i ++){
		block_write(i+1, (void*)fat + (i * BLOCK_SIZE));
	}
	/*write directory*/
	block_write((superblock->num_FAT_blocks)+1,(void*)root_dir);

   /*clear file descriptor*/
   for(int i = 0; i < FILE_OPEN_MAX; i++){
   	file_descriptors[i].isUsed = false;
   }
	printf("umount_fs successfully\n");
	close_disk();
   return 0;
}

/*this additional function helps to find the index of the file with given name*/
int find_file_index(char *name){
	int index = -1;
	for(int i = 0; i < FILE_NUM_MAX; i++){
		if(strcmp(root_dir[i].fileName,name) == 0){
			index = i;
			break;
		}
	}
	return index;
}


/*this additional function helps to find the available file descriptor. Return the index of the available file descriptor.
  Return -1 if none is available
*/
int find_unused_fildes(){
	int index = -1;
	for(int i = 0; i < FILE_OPEN_MAX; i ++){
		if(file_descriptors[i].isUsed == false){
			index = i;
			break;
		}
	}
	return index;
}

/*additional function helps to get number of available fat entries*/
int num_free_entries(){
	int counter = 0;
	for(int i = 0; i < superblock -> num_data_blocks; i++){
		if(fat[i].ind_entry == EMPTY){
			counter ++;
		}
	}
	return counter;
}

/*addtional function helps to get the current FAT entry, for writing and reading*/

int cur_fat_entry(int fat_index, int iteration){
	for(int i = 0; i < iteration; i ++){
		if(fat_index == END_OF_FILE){
			return -1;
		}
		fat_index = fat[fat_index].ind_entry;
	}
	return fat_index;
}

//file operations

int fs_open(char *name){
	int fildes_index = -1;
	//look for file index using given name, and check if file is already opened
	int index = find_file_index(name);
	if(index == -1) return -1;
   if(root_dir[index].isActive == true){
   	//look for file descriptor associated with the file and return the value
   	for(int i = 0; i < FILE_OPEN_MAX ; i++){
   		if(file_descriptors[i].ind == index){
   			fildes_index = i;
   			break;
   		}
   	}
   }
   printf("//======fs_open======//\n");
   printf("file descriptor index of %s is %d\n", name, fildes_index);
   printf("\n");
   //return -1 if none file descriptor is available
   //else, initialize the available file descriptor and activate/open file.
   if(find_unused_fildes() == -1){
   	return -1;
   }else{
   	fildes_index = find_unused_fildes();
   	file_descriptors[fildes_index].ind = index;
   	file_descriptors[fildes_index].offset = 0;
   	file_descriptors[fildes_index].isUsed = true;
   	strcpy(file_descriptors[fildes_index].fileName,name);
   	root_dir[index].isActive = true;
   }
	return fildes_index;
}

int fs_close(int fildes){
	if(fildes < 0 || fildes >31) return -1;
	if(file_descriptors[fildes].isUsed == false) return -1;
   
   file_descriptors[fildes].isUsed = false;
   int index_file = file_descriptors[fildes].ind;
   root_dir[index_file].isActive = false;

	return 0;
}

int fs_create(char *name){
	if(strlen(name) > FILENAME_LEN_MAX) return -1;

	if(find_file_index(name) != -1) return -1;

	for(int i = 0; i < FILE_NUM_MAX; i ++){
		if(root_dir[i].isActive == false){
			root_dir[i].file_size = 0;
			strcpy(root_dir[i].fileName,name);
			root_dir[i].isActive = true;
			root_dir[i].first_data_block = END_OF_FILE;
			printf("//======fs_create()======//\n");
			printf("Create %s\n",root_dir[i].fileName);
			printf("root_dir[%d].file_size = %d\n",i, root_dir[i].file_size);
			printf("root_dir[%d].isActive = %d\n",i, root_dir[i].isActive);
			printf("root_dir[%d].first_data_block = %d\n",i, root_dir[i].first_data_block);
			printf("\n");
			return 0;
		}
	}

	return -1;
}

int fs_delete(char *name){
	int index_file = find_file_index(name);
	if(index_file == -1) return -1;
	if(root_dir[index_file].isActive == true) return -1;

	
	//remove file information
	//free blocks which contain the file data
	struct rootDirectory* dir = &root_dir[index_file];
	int first_data_block = dir->first_data_block;
	while(first_data_block != END_OF_FILE){
		int tmp = fat[first_data_block].ind_entry;
		fat[first_data_block].ind_entry = EMPTY;
		first_data_block = tmp;
	}
	
	memset(dir->fileName, 0, FILENAME_LEN_MAX);
	dir->file_size = 0;
	dir -> isActive = false;	
	return 0;
}



int fs_read(int fildes, void *buf, size_t nbyte){
	if(fildes < 0 || fildes < FILE_OPEN_MAX || file_descriptors[fildes].isUsed == false || nbyte <= 0) return -1;
	}

	//get all the file information to prep for file write
  //file name and file index of the file the file descriptor is associated with
  //the offset of the file descriptor
  //number of data blocks that have the content of the file
  //the block location of current offset 
  

	char *fileName = file_descriptors[fildes].fileName;
	off_t offset = file_descriptors[fildes].offset;
	int file_index = find_file_index(fileName);
  struct rootDirectory *dir = &root_dir[file_index];
  int file_size = dir->file_size;
  
  //check if nbytes can cause greater-than-EOF issue.
  //read til the EOF if it is the issue

  int nbytes_to_read = 0;
  if(offset + nbyte > dir->file_size){
   	printf("s_read(): offset + nbytes is greater than file size");
   	nbytes_to_read += abs(file_size - offset);
  }else{
  	nbytes_to_read = nbyte;
  	printf("fs_read(): offset + nbytes is less than/equal to file size\n");
  }

  int fat_iteration = dir -> first_data_block;
  int num_blocks = (nbytes_to_read / BLOCK_SIZE) + 1; // in case amount is 0, read the first block which is EOF

   //get current data block and current location in that data block
  int cur_block    = offset / BLOCK_SIZE;
  int cur_location = offset % BLOCK_SIZE;
  char buf_b[BLOCK_SIZE];

   //go to cur entry
  fat_iteration = cur_fat_entry(fat_iteration, cur_block);

   //read 
  int shift = 0;
  int total_read = 0;
  for(int i = 0; i <num_blocks;i++){
   	if(cur_location + nbytes_to_read > BLOCK_SIZE){
   		shift = BLOCK_SIZE - cur_location;
   	}else{
   		shift = nbytes_to_read;
    }

   	//read content from disk
  	block_read(fat_iteration + superblock->ind_start_data_block, (void*) buf_b);
  	memcpy(buf, buf_b + cur_location, shift);

      //update total of bytes read
  		total_read += shift;
  		buf += shift;
  		cur_location = 0;
      fat_iteration = fat[fat_iteration].ind_entry;
      nbytes_to_read -= shift;
  }

  file_descriptors[file_index].offset += total_read;
  printf("//======fs_read()======//\n");
  printf("File name = %s\n", file_descriptors[fildes].fileName);
  printf("The number of read bytes: %d\n",total_read);
	return total_read;
}

int fs_write(int fildes, void *buf, size_t nbyte){
	if(nbyte <= 0 || fildes < 0 || fildes >= 32) return -1;
	if(num_free_entries == 0) return -1;
  if (file_descriptors[fildes].isUsed == false) return -1;

  //get all the file information to prep for file write
  //file name and file index of the file the file descriptor is associated with
  //the offset of the file descriptor
  //number of data blocks that have the content of the file
  //the block location of current offset 
  //the entry index which maps to the first data block of the file;

  char *fileName = file_descriptors[fildes].fileName;
  int file_index = find_file_index(fileName);
  int offset = file_descriptors[fildes].offset; 

  struct rootDirectory *dir = &root_dir[file_index];
  int cur_num_blocks_file = ((nbyte + (offset % BLOCK_SIZE)) / BLOCK_SIZE) + 1; 
  int cur_block_file = offset / BLOCK_SIZE;
  int cur_fat_index = dir -> first_data_block;

  //Iterate through blocks
  char *write_buf = (char*)buf;
  char bounce_buf[BLOCK_SIZE];
  int amount_to_write = nbyte;
  int available_nbytes; //available unused space of the current block
  int total_byte_written = 0;
  int location = offset % BLOCK_SIZE;

  //go to the starting block
  cur_fat_index = cur_fat_entry(cur_fat_index, cur_block_file);
  int available_data_blocks = 0;
  int fat_block_indices[cur_num_blocks_file];

  //locate and store indices of the free blocks
  //to avoid overwriting other file contents
  printf("fs_write(): num_data_blocks = %d\n", superblock->num_data_blocks);
  for(int i = 0; i < superblock->num_data_blocks; i ++){
  	if(fat[i].ind_entry == 0){
  		fat_block_indices[available_data_blocks] = i;
  		available_data_blocks ++;
  		printf("fs_write(): fat_block_indices[%d] = %d\n",available_data_blocks - 1, i );
  	}else{
  		printf("fs_write(): ind_entry=%d\n", fat[i].ind_entry);
  	}
  	if(available_data_blocks == cur_num_blocks_file){
  		break;
  	}
  }

  //write the disk
  int num_free = num_free_entries();
  if(cur_num_blocks_file > num_free){
  	cur_num_blocks_file = num_free;
  }

  //iterate to write 

  for(int i = 0; i < cur_num_blocks_file; i ++){
  	if(location + amount_to_write > BLOCK_SIZE){
  		available_nbytes = BLOCK_SIZE - location;
  	}else{
  		available_nbytes = amount_to_write;
  	}
  	//continue to write at the current offset
  	memcpy(bounce_buf + location, write_buf, available_nbytes);
  	block_write(cur_fat_index + superblock->ind_start_data_block, (void*)bounce_buf);

  	//update the process with total numer of bytes written
  	//move the pointer of write_buf to move on to the next nbytes which are not written yet
  
  	total_byte_written += available_nbytes;
  	write_buf += available_nbytes;
  	location = 0;
  	amount_to_write -= available_nbytes;

  	// updating the final FAT entry values 
			if(i < cur_num_blocks_file - 1){
				fat[cur_fat_index].ind_entry = fat_block_indices[i+1];
				cur_fat_index = fat[cur_fat_index].ind_entry;
			}
			else{
				fat[cur_fat_index].ind_entry = END_OF_FILE;
				cur_fat_index = fat[cur_fat_index].ind_entry;
			}
  	}

  

			// update filesize accordingly to how much was written 
	if(offset + total_byte_written > dir->file_size){
			dir->file_size = offset + total_byte_written;
	}

	file_descriptors[fildes].offset += total_byte_written;
	return total_byte_written;
}

int fs_get_filesize(int fildes){
	if(fildes < 0 || fildes > FILE_OPEN_MAX || file_descriptors[fildes].isUsed == false){
		return -1;
	}

	struct fileDescriptor *fd = &file_descriptors[fildes];
   int index_file = find_file_index(fd->fileName);
   if(index_file == -1) return -1;

	return root_dir[index_file].file_size;
}

int fs_lseek(int fildes, off_t offset){
	if(file_descriptors[fildes].isUsed == false) return -1;
	if(offset < 0 || offset > fs_get_filesize(fildes)) return -1;

	file_descriptors[fildes].offset = offset;
	return 0;
}

int fs_truncate(int fildes, off_t length){
	if(file_descriptors[fildes].isUsed == false) return -1;
	if(length < 0 || length > fs_get_filesize(fildes)) return -1;
	/*
	still in process, not completed

	*/
	return -1;
}


     
// int main(void){
// 	 int rtn, fd;
//     char wt[11];
//     char rd[11];

//     memset(wt, 0, 11);
//     memset(rd, 0, 11);
//     memset(wt, 'a', 10);

//     make_fs ("disk.4");
//     mount_fs("disk.4");

//     fs_create("file.4");
//     fd = fs_open("file.4");

//     int nbytes_written = fs_write(fd, wt, 10);
//     printf(" nbytes written: %d\n",nbytes_written);
//     rtn = fs_read(fd, rd, 10);
//     if (rtn != 0){
//         strcat(str,"rtn!=0\n");
//         printf("FAIL fs_read() rtn !=0\n");  
//     }
        

//     rtn = fs_lseek(fd, 0);
//     if (rtn){
//         strcat(str,"rtn = fs_Lseek\n");
//         printf("FAIL fs_lseek()\n");
//     }
       

//     rtn = fs_read(fd, rd, 10);

//     if (rtn != 10){
//         strcat(str,"rtn != 10\n");
//         printf("FAIL fs_read() rtn!=10 \n");
//     }

//     if (strcmp(wt, rd)){
//         strcat(str,"wr != rd\n");
//         printf("FAIL wr!=rd\n");
//     }


//     fs_close(fd);
//     umount_fs("disk.4");
// }