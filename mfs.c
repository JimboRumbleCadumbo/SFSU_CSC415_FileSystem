/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: mfs.c
*
* Description:: 
*	This is the file system interface.
*	This is the interface needed by the driver to interact with
*	your filesystem.
*
**************************************************************/

#include "mfs.h"

// Key directory functions
int fs_mkdir(const char *pathname, mode_t mode){
	return 0;
}
int fs_rmdir(const char *pathname){
	return 0;
}

// Directory iteration functions
fdDir * fs_opendir(const char *pathname);

struct fs_diriteminfo *fs_readdir(fdDir *dirp){
	return dirp;
}
int fs_closedir(fdDir *dirp){
    return dirp;
}

// Misc directory functions
char * fs_getcwd(char *pathname, size_t size){
    return 0;
}

int fs_setcwd(char *pathname){ //linux chdir
    return 0;
}

int fs_isFile(char * filename){ //return 1 if file, 0 otherwise
    struct fs_diriteminfo dirEntry;

    if (fetchDirEntry(filename, &dirEntry) != 0) {
        return 0;
    }
    // Not ready yet
    // return dirEntry.fileType == ...;
}	

int fs_isDir(char * pathname){ //return 1 if directory, 0 otherwise
    struct fs_diriteminfo dirEntry;

    if (fetchDirEntry(pathname, &dirEntry) != 0) {
        return 0;
    }
    // Not ready yet
    // return dirEntry.fileType == ...;
}

// Function to retrieve file or directory data
int fs_stat(const char *path, struct fs_stat *buf) {
    struct fs_diriteminfo dirEntry;

    if (fetchDirEntry(path, &dirEntry) != 0) {
        return -1; // Invalid path
    }

    memset(buf, 0, sizeof(struct fs_stat));  // Clear the struct
    buf->st_size = dirEntry.d_reclen;  // Assumed the size matches d_reclen
    buf->st_blksize = 512;
    buf->st_blocks = (buf->st_size + 511) / 512;

    buf->st_accesstime = time(NULL);  
    buf->st_modtime = time(NULL);
    buf->st_createtime = time(NULL);

    // buf->st_mode = (dirEntry.fileType == ...);

    return 0;
}

int fs_delete(char* filename){ //removes a file
    return 0;
}	