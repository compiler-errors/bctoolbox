/*
bc_vfs.h
Copyright (C) 2016 Belledonne Communications SARL

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef bc_vfs_h
#define bc_vfs_h
#include <fcntl.h>

#include <bctoolbox/port.h>
#include <stdbool.h>


#if !defined(_WIN32_WCE)
#include <sys/types.h>
#include <sys/stat.h>
#if _MSC_VER
#include <io.h>
#endif
#endif /*_WIN32_WCE*/


#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef _WIN32	

#ifndef S_IRUSR
#define S_IRUSR S_IREAD
#endif

#ifndef S_IWUSR
#define S_IWUSR S_IWRITE
#endif

#define open _open
#define read _read
#define write _write
#define close _close
#define lseek _lseek

#endif /*!_WIN32*/

#define BCTBX_VFS_OK           0   /* Successful result */

#define BCTBX_VFS_ERROR       -255   /* Some kind of disk I/O error occurred */


/**
 * Methods associated with the bctbx_vfs_t.
 */
typedef struct bctbx_io_methods bctbx_io_methods;

/**
 * VFS file handle.
 */
typedef struct bctbx_vfs_file_t bctbx_vfs_file_t;
struct bctbx_vfs_file_t {
	const struct bctbx_io_methods *pMethods;  /* Methods for an open file: all implementors must supply this field at open step*/
	/*the fields below are used by the default implementation. Implementors are not required to supply them, but may use them if they find
	 * them useful*/
	uint64_t size;					/*size of file*/
	int fd;                         		/* File descriptor */
	off_t offset;								/*File offset used by lseek*/
};


/**
 */
struct bctbx_io_methods {
	int (*pFuncClose)(bctbx_vfs_file_t *pFile);
	int (*pFuncRead)(bctbx_vfs_file_t *pFile, void* buf, int count, uint64_t offset);
	int (*pFuncWrite)(bctbx_vfs_file_t *pFile , const void* buf, int count, uint64_t offset );
	int (*pFuncFileSize)(bctbx_vfs_file_t *pFile);
	int (*pFuncGetLineFromFd)(bctbx_vfs_file_t *pFile , char* s, int count);
	int (*pFuncSeek)(bctbx_vfs_file_t *pFile, uint64_t offset, int whence);
};


/**
 * VFS definition
 */
typedef struct bctbx_vfs_t bctbx_vfs_t;
struct bctbx_vfs_t {
	bctbx_vfs_t *pNext;      		/* Next registered VFS */
	const char *vfsName;       /* Virtual file system name */
	int (*pFuncFopen)(bctbx_vfs_t* pVfs, bctbx_vfs_file_t *pFile, const char *fName,  const int openFlags);

	
};


/* API to use the VFS */
/*
 * This function returns a pointer to the VFS implemented in this file.
 */
bctbx_vfs_t *bc_create_vfs(void);
bctbx_vfs_t *bctbx_vfs_find(bctbx_vfs_t* p, const char *zVfsName);

/**
 * Assigns the VFS pointer in use pVfs to pToVfs.
 * @param  pVfs   Pointer to the vfs instance in use.
 * @param  pToVfs Pointer to the vfs pointer in use.
 * @return       BCTBX_VFS_ERROR if pVfs is NULL,  BCTBX_VFS_OK otherwise.
 */
int bctbx_vfs_register(bctbx_vfs_t* pVfs, bctbx_vfs_t** pToVfs);

/**
 * Attempts to read count bytes from the open file given by pFile, at the position starting at offset 
 * in the file and and puts them in the buffer pointed by buf.
 * @param  pFile  bctbx_vfs_file_t File handle pointer.
 * @param  buf    Buffer holding the read bytes.
 * @param  count  Number of bytes to read. 
 * @param  offset Where to start reading in the file (in bytes).
 * @return        Number of bytes read on success, BCTBX_VFS_ERROR otherwise. 
 */
int bctbx_file_read(bctbx_vfs_file_t* pFile, void *buf, int count, uint64_t offset);

/**
 * Close the file from its descriptor pointed by thw bctbx_vfs_file_t handle. 
 * @param  pFile File handle pointer.
 * @return      	return value from the pFuncClose VFS Close function on success, 
 *                  BCTBX_VFS_ERROR otherwise. 
 */
int bctbx_file_close(bctbx_vfs_file_t* pFile);

/**
 * Allocates a bctbx_vfs_file_t file handle pointer. Opens the file fName
 * with the mode specified by the mode argument. Calls bctbx_file_open.
 * @param  pVfs  Pointer to the vfs instance in use.
 * @param  fName Absolute file path.
 * @param  mode  File access mode (char*).
 * @return  pointer to  bctbx_vfs_file_t on success, NULL otherwise.      
 */
bctbx_vfs_file_t* bctbx_file_open(bctbx_vfs_t* pVfs, const char *fName,  const char* mode);
/**
 * Allocates a bctbx_vfs_file_t file handle pointer. Opens the file fName
 * with the mode specified by the mode argument. Calls bctbx_file_open.
 * @param  pVfs  		Pointer to the vfs instance in use.
 * @param  fName 		Absolute file path.
 * @param  openFlags  	File access flags(integer).
 * @return  pointer to  bctbx_vfs_file_t on success, NULL otherwise.      
 */
bctbx_vfs_file_t* bctbx_file_open2(bctbx_vfs_t* pVfs, const char *fName,  const int openFlags );


/**
 * Returns the file size.
 * @param  pFile  bctbx_vfs_file_t File handle pointer.
 * @return       BCTBX_VFS_ERROR if an error occured, file size otherwise. 
 */
uint64_t bctbx_file_size(bctbx_vfs_file_t *pFile);

/**
 * Write count bytes contained in buf to a file associated with pFile at the position
 * offset. Calls pFuncWrite (set to bc_Write by default).
 * @param  pFile 	File handle pointer.
 * @param  buf    	Buffer hodling the values to write.
 * @param  count  	Number of bytes to write to the file.
 * @param  offset 	Position in the file where to start writing. 
 * @return        	Number of bytes written on success, BCTBX_VFS_ERROR if an error occurred.
 */
int bctbx_file_write(bctbx_vfs_file_t* pFile, const void *buf, int count, uint64_t offset);

/**
 * Writes to file. 
 * @param  pFile  File handle pointer.
 * @param  offset where to write in the file
 * @param  fmt    format argument, similar to that of printf
 * @return        Number of bytes written if success, BCTBX_VFS_ERROR otherwise.
 */
int bctbx_file_fprintf(bctbx_vfs_file_t* pFile, uint64_t offset, const char* fmt, ...);

/**
 * Wrapper to pFuncGetNxtLine. Returns a line with at most maxlen characters 
 * from the file associated to pFile and  writes it into s.
 * @param  pFile  File handle pointer.
 * @param  s      Buffer where to store the read line.
 * @param  maxlen Number of characters to read to find a line in the file. 
 * @return        BCTBX_VFS_ERROR if an error occurred, size of line read otherwise. 
 */
int bctbx_file_get_nxtline(bctbx_vfs_file_t* pFile, char*s , int maxlen);

/**
 * Wrapper to pFuncSeek VFS method call. Set the position to offset in the file.
 * @param  pFile  File handle pointer.
 * @param  offset File offset where to set the position to.
 * @param  whence Either SEEK_SET, SEEK_CUR,SEEK_END 
 * @return        BCTBX_VFS_ERROR on error, offset otherwise. 
 */
int bctbx_file_seek(bctbx_vfs_file_t *pFile, uint64_t offset, int whence);


/**
 * Find the VFS in use within the VFS table.
 * @param  p        bctbx_vfs pointer to the first VFS
 * @param  zVfsName VFS name
 * @return          pointer to the VFS with the name zVfsName if found, NULL otherwise.
 */
bctbx_vfs_t *bctbx_vfs_find(bctbx_vfs_t* p, const char *zVfsName);

const bctbx_io_methods* get_bcio(void);


void bctbx_vfs_set_default(bctbx_vfs_t *my_vfs);
bctbx_vfs_t * bctbx_vfs_get_default();

#endif

