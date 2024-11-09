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
        return -1; // Empty path
    }
    DE *retParent; 
    int index = 0;
    char lastElemName[MAX_NAME_LENGTH];
    char *path = strdup(pathname);
    int result = parsePath(path, &retParent, &index, lastElemName);
    free(path);
    path = NULL;
    if (result < 0) {
        return -1; // Invalid path / error during parsing
    }
    printf("Finished parsing path. %p is the root; %p is retParent.\n", root, retParent);
    DE *loadedDir = loadDir(retParent);
    if (loadedDir == NULL) {
        free(retParent);
        printf("Error loading directory.\n");
        return -1; // error loading
    }
    int idx = firstUnusedDirEntry(loadedDir);
    printf("The first unused directory entry in the parent directory is %d.\n", idx);
    DE *newDir = createDirectory(ENTRIES_IN_DIR, loadedDir);
    if (newDir == NULL) {
        printf("Creating directory failed.\n");
        return -1;
    }
    strcpy(loadedDir[idx].name, lastElemName);
    loadedDir[idx].isDirectory = 1;
    loadedDir[idx].location = newDir->location;
    loadedDir[idx].size = newDir->size;
    loadedDir[idx].timeCreated = newDir->timeCreated;
    loadedDir[idx].timeModified = newDir->timeModified;
    if (writeDir(loadedDir) < 1) {
        printf("Failed to write directories.\n");
        return -1; 
    }
    printf("Success.\n");
    return 0;
}

int fs_rmdir(const char *pathname){
	return 0;
}

// Directory iteration functions
fdDir * fs_opendir(const char *pathname) {
    printf("Opening directory %s.\n", pathname);
    if (pathname == NULL || strlen(pathname) == 0) {
        return NULL; // Invalid path
    }
    printf("Parsing path.\n");
    DE *retParent;
    int index = 0; 
    char lastElemName[MAX_NAME_LENGTH]; 
    // Copy path to pass into parsePath so we can strtok it
    char *path = strdup(pathname);
    int result = parsePath(path, &retParent, &index, lastElemName);
    printf("Finished parsing path.\n");
    free(path);
    path = NULL;
    if (result < 0) {
        return NULL; // Invalid path
    }
    if (retParent == NULL || index < 0 || lastElemName == NULL) {
        return NULL; // Error cases
    }
    fdDir *openedDir;
    openedDir = (fdDir *)malloc(sizeof(fdDir));
    if (openedDir == NULL) {
        free(path);
        path = NULL;
        return NULL; // Error malloc
    }
    DE *directory; 
    directory = loadDir(&retParent[index]);
    struct fs_diriteminfo *dirInfo;
    dirInfo = (struct fs_diriteminfo *)malloc(sizeof(struct fs_diriteminfo));
    if (dirInfo == NULL) {
        free(directory);
        directory = NULL;
        free(openedDir);
        openedDir = NULL;
        free(path);
        path = NULL;
        return NULL; // Malloc failed
    }
    openedDir->d_reclen = sizeof(fdDir);
    openedDir->dirEntryPosition = 0;
    openedDir->directory = directory;
    openedDir->di = dirInfo;
    return openedDir;
}


struct fs_diriteminfo *fs_readdir(fdDir *dirp) {
    
    if(dirp->dirEntryPosition < 50){
        dirp->di->d_reclen = sizeof(struct fs_diriteminfo);

        dirp->di->timeCreated = dirp->directory->timeCreated;
        dirp->di->timeModified = dirp->directory->timeModified;

        strncpy(dirp->di->d_name, dirp->directory[dirp->dirEntryPosition].name, MAX_NAME_LENGTH);
        dirp->dirEntryPosition++;

    }

    return dirp->di;
}


int fs_closedir(fdDir *dirp) {
    free(dirp->di);
    dirp->di = NULL;
    free(dirp->directory);
    dirp->directory = NULL;
    
    dirp->d_reclen = 0;
    dirp->dirEntryPosition = 0;

    free(dirp);
    dirp = NULL;
    return 0; // Close success
}

// Misc directory functions
char * fs_getcwd(char *pathname, size_t size){
    // Copy the current working directory string to the provided buffer
    strncpy(pathname, cwdString, size - 1);
    pathname[size - 1] = '\0'; // Ensure null-termination

    return pathname; // Return the buffer containing the current working directory
}

int fs_setcwd(char *pathname) { //linux chdir
    printf("Starting set CWD function.\n");
    if (pathname == NULL || strlen(pathname) == 0) {
        printf("Empty path.\n");
        return -1; // Empty path 
    }

    // Validate the input path & confirm last element exists
    DE *retParent;
    int index = 0;
    char lastElemName[MAX_NAME_LENGTH];
    int result = parsePath(pathname, &retParent, &index, lastElemName);
    if (index == -1 || retParent == NULL) {
        printf("Index is -1 or retParent is null.\n");
        return -1; // Safety check
    }
    if (result == -1 || retParent[index].isDirectory == 0) {
        printf("Invalid path or directory not found or not directory.\n");
        return -1; // Invalid path or not directory or directory not found
    }
    printf("Loading directory..\n");
    DE *temp = loadDir(&retParent[index]);
    if (temp < 0) {
        printf("Error loading directory.\n");
        return -1;
    }
    // Free the previous cwd
    if (cwd != root) {
        printf("Freeing previous CWD.\n");
        free(cwd);
    }
    strcpy(cwd->name, lastElemName);
    printf("The new CWD is %s.\nAttributes: \nlocation: %d, time created: %s\n", cwd->name, cwd->location, ctime(&cwd->timeCreated));
    // Update the cwdString
    printf("Updating CWD path string\n");
    char newCwdString[MAX_PATH_LENGTH];
    if (pathname[0] == '/') {
        // Absolute path
        strncpy(newCwdString, pathname, MAX_PATH_LENGTH - 1);
        newCwdString[MAX_PATH_LENGTH - 1] = '\0';
    } else {
        // Relative path
        snprintf(newCwdString, MAX_PATH_LENGTH, "%s%s", cwdString, pathname);
    }

    // Normalize the path
    char *token;
    char *token2;
    char *rest = newCwdString;
    char *saveptr;
    char *tokens[MAX_PATH_LENGTH];
    int tokenCount = 0;


    token = strtok_r(rest, "/", &saveptr);
    while (token != NULL) {
        printf("starting while loop\n");
        if (strcmp(token, ".") == 0) {
            continue; // Ignore "."
        } else if (strcmp(token, "..") == 0) {
            if (tokenCount > 0) {
                tokenCount--; // Go back one directory
            }
        } else {
            tokens[tokenCount] = token; // Add to tokens
            tokenCount++;
        }
        token = strtok_r(NULL, "/", &saveptr);
    }
    // Create an array of integers that matches the number of entries in the vector
    int indices[tokenCount];
    int idx = 0;

    // Loop through the vector
    for (int i = 0; i < tokenCount; i++) {
        strcat(cwdString, tokens[i]);
    }

    // Null terminate cwdString
    newCwdString[strlen(newCwdString)] = '\0';
    strncpy(cwdString, newCwdString, strlen(newCwdString));

    printf("New CWD string: %s\n", cwdString);
    return 0; // Success
}

int fs_isFile(char * filename){
    DE *retParent;
    int index = 0; 
    char lastElemName[MAX_NAME_LENGTH]; 
    int result = parsePath(filename, &retParent, &index, lastElemName);
    if (result < 1 || index < 0 || lastElemName == NULL || retParent == NULL) {
        return -1; // Failure
    }
    if (strcmp(retParent[index].name, lastElemName) != 0) {
        return -1; // Names don't match
    }
    if (retParent[index].isDirectory == 1) {
        return 0;
    }
    if (retParent[index].isDirectory == 0) {
        return 1;
    }
}	

int fs_isDir(char * pathname){
    DE *retParent;
    int index = 0; 
    char lastElemName[MAX_NAME_LENGTH]; 
    int result = parsePath(pathname, &retParent, &index, lastElemName);
    if (result < 1 || index < 0 || lastElemName == NULL || retParent == NULL) {
        return -1; // Failure
    }
    if (strcmp(retParent[index].name, lastElemName) != 0) {
        return -1; // Names don't match
    }
    if (retParent[index].isDirectory == 1) {
        return 1;
    }
    if (retParent[index].isDirectory == 0) {
        return 0;
    }
}

// Function to retrieve file or directory data
int fs_stat(const char *path, struct fs_stat *buf) {
    if (path == NULL || strlen(path) == 0) {
        return -1;
    }
    DE *retParent;
    int index = 0; 
    char lastElemName[MAX_NAME_LENGTH]; 
    // Copy path to pass into parsePath so we can strtok it
    char *pathname = strdup(path);
    int result = parsePath(pathname, &retParent, &index, lastElemName);
    free(pathname);
    pathname = NULL;
    if (result < 0) {
        return -1; // Invalid path
    }
    if (retParent == NULL || lastElemName == NULL || index < 0) {
        return -1; // No parent / no last element name / dir does not exist in parent
    }
    buf->st_size = retParent[index].size;  // Assumed the size matches d_reclen
    buf->st_blksize = vcb->blockSize;
    buf->st_blocks = (buf->st_size + (vcb->blockSize - 1)) / (vcb->blockSize);

    buf->st_createtime = retParent[index].timeCreated;
    buf->st_modtime = retParent[index].timeModified;

    // We aren't tracking access time in our DE struct (structs.h)
    // buf->st_accesstime = time(NULL); // Placeholder

    // 1 for Directory, 0 for File
    buf->st_mode = retParent[index].isDirectory;
    return 0;
}

int fs_delete(char* filename){ //removes a file
    return 0;
}	