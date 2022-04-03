/** Maximum length for a file name **/
#define FILENAME_LEN_MAX 15

/** Maximum number of files in the directory **/
#define FILE_NUM_MAX 64

/** Maximum of 32 file descriptors **/
#define FILE_OPEN_MAX 32
/** 
 * function make_fs
 * @disk_name
 * 
 * Create a fresh (and empty) file system on the virtual disk
 * with name disk_name. 
 * 
 * As part of this function, you should first invoke make_disk(disk_name)
 * to create a new disk.
 * 
 * This function returns 0 on success, and -1 when the disk disk_name 
 * could not be created, opened, or properly initilized
 * **/
int make_fs(char *disk_name);


/** 
 * function mount_fs
 * @disk_name
 * 
 * Mount a file system that is stored on a virtual disk with name
 * disk_name. With the mount operation, a file system becomes "ready
 * for use". 
 * 
 * This function returns 0 on sucess, and -1 when the disk disk_name could
 * not be opened or when the disk does not contain a valid file system (that
 * you previously created with make_fs).
 * **/

int mount_fs(char *disk_name);

/** 
 * function umount_fs
 * @disk_name
 * 
 * Unmount your file system from a virtual disk with name disk_name. 
 * 
 * The function returns 0 on success, and -1 when the disk disk_name
 * could not be closed or when data could not be written to the disk (this 
 * should not happen).
 * **/

int umount_fs(char *disk_name);

/** 
 * function fs_open
 * 
 * @name
 * 
 * The file specified by name is opened for reading and writing, and
 * the file descriptor corresponding to this file is returned to the calling 
 * function.
 * 
 * The function returns a non-negative integer on success. The returned number
 *  can be used to subsequently access this file. Return -1 on failure when
 * the file with name cannot be found, or when there are already 32 file descriptors
 * active
 * 
 * When a file is opened, the file offset (seek pointer) is set to 0 (the beginning
 * of the file)
 * **/

int fs_open(char *name);

/**
 * function fs_close
 * 
 * @fildes
 * 
 * The file descriptor fildes is closed. 
 * 
 * Return 0 on success, and return -1 when the file descriptor fildes does not exist
 * or is not open
 *  **/

int fs_close(int fildes);

/** 
 * function fs_create
 * 
 * @name
 * 
 * This function creates a new file with name name in the root directory of your file 
 * system. The file is initially empty. The maximum length for a file name is 15 characters.
 * 
 * 
 * At most 64 files in the directory
 * 
 * Return 0 on success, and return -1 on failure when the file with name already exists.
 * or when the file name is too long, or when there are already 64 files present in the 
 * root directory
 * **/

int fs_create(char *name);

/** 
 * function fs_delete
 * 
 * @name
 * 
 *Delete the file with name name from the root directory of your file system and frees all data 
 *blocks and meta-information that correspond to that file. The file that is being deleted must 
 *not be open.
 *
 *Return 0 success, and return -1 on failure when the file is not deleted   
 * **/

int fs_delete(char *name);

/** 
 * function fs_read
 * 
 * @fildes
 * 
 * @buf
 * 
 * @nbyte
 * 
 * Attempt to read nbyte bytes of data from the file referenced by the descriptor
 * fildes into the buffer pointed to by buf.
 * 
 * Return number of bytes until the end of the file on success, and return -1 on failure
 * **/

int fs_read(int fildes, void *buf, size_t nbyte);

/** 
 * function fs_write
 * 
 * @fildes
 * 
 * @buf
 * 
 * @nbyte
 * 
 * Attempt to write nbyte bytes of data to the file referenced by the descriptor fildes
 * from the buffer pointed to by buf. 
 * 
 * When the function attempts to write past the end of the file, the file is automatically
 * extended to hold the additional bytes
 * 
 * The maximum file size is 16M
 * 
 * Return number of bytes that were actually written on success, and return -1 on failure
 * **/

int fs_write(int fildes, void *buf, size_t nbyte);

/** 
 * function fs_get_filesize
 * 
 * @fildes
 * 
 * @offset
 * 
 * Return the current size of the file pointed to by the file descriptor fildes.
 * 
 * Return -1 if fildes is invalid
 *  
 * **/

int fs_get_filesize(int fildes);

/** 
 * function fs_sleek
 * 
 * @fildes
 * 
 * @offset
 * 
 * Set the file pointer (the offset used for read and write operations) associated
 * with the file descriptor fildes to the argument offset.
 * 
 * Err if the file pointer is set beyound the EOF
 * 
 * Return 0 on success, and return -1 on failure when fildes is invalid, or the requested
 * offset is larger than the file size, or when offset is less than 0
 * 
 * **/

int fs_lseek(int fildes, off_t offset);

/** 
 * function fs_truncate
 * 
 * @filder
 * 
 * @length
 * 
 * Cause the file referenced by the fildes to be truncated to length bytes in size.
 * 
 * Return 0 on success, and return -1 on failure when the file descriptor fildes is invalid
 * or the requested length is larger than the file size
 * 
 * **/
int fs_truncate(int fildes, off_t length);