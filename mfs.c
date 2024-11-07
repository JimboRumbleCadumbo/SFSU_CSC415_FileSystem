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

int fs_isFile(char * filename){
    struct fs_diriteminfo entry;

    if (fetchDirEntry(filename, &entry) != 0) {
        return 0; // Entry not found or invalid path
    }
    
    // Can replace 0 with FT_REGFILE
    return entry.fileType == 0;
}	

int fs_isDir(char * pathname){
    struct fs_diriteminfo entry;

    if (fetchDirEntry(pathname, &entry) != 0) {
        return 0; // Entry not found or invalid path
    }

    // Can replace 1 with FT_DIRECTORY
    return entry.fileType == 1;
}

// Function to retrieve file or directory data
int fs_stat(const char *path, struct fs_stat *buf) {
    struct fs_diriteminfo entry;

    if (fetchDirEntry(path, &entry) != 0) {
        return -1; // Invalid path
    }

    // Remember to add it into fs_diriteminfo
    memset(buf, 0, sizeof(struct fs_stat));  // Clear the struct
    buf->st_size = entry.d_reclen;  // Assumed the size matches d_reclen
    buf->st_blksize = 512;
    buf->st_blocks = (buf->st_size + 511) / 512;

    buf->st_createtime = entry.timeCreated;
    buf->st_modtime = entry.timeModified;

    // We aren't tracking access time in our DE struct (structs.h)
    // buf->st_accesstime = time(NULL); // Placeholder

    // 1 for Directory, 0 for File
    buf->st_mode = (entry.fileType == 1) ? 1 : 0;

    return 0;
}

int fs_delete(char* filename){ //removes a file
    return 0;
}	