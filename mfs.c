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
	if (pathname == NULL || strlen(pathname) == 0) {
        return -1;
    }
    DE *retParent;
    int index = 0; 
    char *lastElemName; 
    // Copy path to pass into parsePath so we can strtok it
    char *path;
    strncpy(path, pathname, strlen(pathname));
    int result = parsePath(path, retParent, &index, lastElemName);
    if (result < 0) {
        return -1; // Invalid path
    }
    if (retParent == NULL || lastElemName == NULL || index != 0) {
        return -1; // No parent / no last element name / dir exists in parent
    }
    // Load the parent directory 
    DE *loadedParent = loadDir(retParent);
    if (loadedParent == NULL) {
        return -1; 
    }
    DE *newDir;
    newDir = createDirectory(ENTRIES_IN_DIR, &loadedParent[0]);
    int idx = firstUnusedDirEntry(loadedParent);
    if (idx < 2) {
        return -1; // Either ., .., or nothing unused
    }
    strcpy(loadedParent[idx].name, lastElemName);
    loadedParent[idx].size = newDir[0].size;
    loadedParent[idx].isDirectory = 1;
    loadedParent[idx].location = newDir[0].location;
    loadedParent[idx].timeCreated = newDir[0].timeCreated;
    loadedParent[idx].timeModified = newDir[0].timeModified;
    if (writeDir(newDir) < 1) {
        return -1; // Error writing new directory
    }
    if (writeDir(loadedParent) < 1) {
        return -1; // Error writing new directory
    }
    free(newDir);
    free(loadedParent);
    return 0;
}
int fs_rmdir(const char *pathname){
	return 0;
}

// Directory iteration functions
fdDir * fs_opendir(const char *pathname) {
    if (pathname == NULL || strlen(pathname) == 0) {
        return NULL; // Invalid path
    }
    DE *retParent;
    int index = 0; 
    char *lastElemName; 
     // Copy path to pass into parsePath so we can strtok it
    char *path;
    strncpy(path, pathname, strlen(pathname));
    int result = parsePath(path, retParent, &index, lastElemName);
    if (result < 0) {
        return NULL; // Invalid path
    }
    if (retParent == NULL || index < 0 || lastElemName == NULL) {
        return NULL; // Error cases
    }
    int idx = findInDir(retParent, lastElemName);
    if (idx < 0) {
        return NULL; // Dir not found in parent
    }
    fdDir *openedDir;
    openedDir = (fdDir *)malloc(sizeof(fdDir));
    if (openedDir == NULL) {
        return NULL; // Error malloc
    }
    DE *directory; 
    directory = loadDir(&retParent[idx]);
    struct fs_diriteminfo *dirInfo;
    dirInfo = (struct fs_diriteminfo *)malloc(sizeof(struct fs_diriteminfo));
    if (dirInfo == NULL) {
        free(directory);
        directory = NULL;
        free(openedDir);
        openedDir = NULL;
        return NULL; // Malloc failed
    }
    openedDir->d_reclen = sizeof(fdDir);
    openedDir->dirEntryPosition = 0;
    openedDir->directory = directory;
    openedDir->di = dirInfo;
    return openedDir;
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp){
	return dirp->di;
}
int fs_closedir(fdDir *dirp){
    return 0;
}

// Misc directory functions
char * fs_getcwd(char *pathname, size_t size){
    // Copy the current working directory string to the provided buffer
    strncpy(pathname, cwdString, size - 1);
    pathname[size - 1] = '\0'; // Ensure null-termination

    return pathname; // Return the buffer containing the current working directory
}

int fs_setcwd(char *pathname){ //linux chdir
    if (pathname == NULL || strlen(pathname) == 0) {
        return -1; // Empty path 
    }
    // Validate the input path & confirm last element exists
    DE *retParent;
    int *index = 0;
    char *lastElemName;
    int result = parsePath(pathname, retParent, index, lastElemName);
    if (index == NULL || retParent == NULL) {
        return -1; // Safety check
    }
    if (result == -1 || retParent[*index].isDirectory == 0 || *index == -1) {
        return -1; // Invalid path or not directory or directory not found
    }

    DE *temp = loadDir(&retParent[*index]);
    // Free the previous cwd
    if (cwd != root) {
        free(cwd);
    }
    cwd = temp; // Update the cwd directory entry

    //TODO: Vector implementation of updating the string

    return 0; // Success
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