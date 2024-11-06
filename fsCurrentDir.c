/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo, riathakker, yyyuquan, Rishita317 
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: fs_setcwd.c
*
* Description:: This file contains the implementation of the 
* fs_setcwd function, which sets the current working directory 
* in the filesystem. This function is crucial for implementing 
* other filesystem operations such as make directory (md), list 
* directory (ls), print working directory (pwd), and change 
* directory (cd).
**************************************************************/

#include "fsCurrentDir.h"

// Function to set the current working directory
int setCwd(const char *path) {
    if (path == NULL || strlen(path) == 0) {
        return -1; // Empty path 
    }
    // Validate the input path & confirm last element exists
    DE *retParent;
    int *index = 0;
    char *lastElemName;
    int result = parsePath(path, retParent, index, lastElemName);
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

// Function to get the current working directory
const char* getCwd() {
    return cwdString;
}