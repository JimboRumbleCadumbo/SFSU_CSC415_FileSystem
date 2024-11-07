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
#include <string.h>
#include <stdlib.h>


// Function to set the current working directory
int setCwd(char *path) {
    if (path == NULL || strlen(path) == 0) {
        return -1; // Empty path 
    }

    // Validate the input path & confirm last element exists
    DE *retParent;
    int index = 0;
    char lastElemName[MAX_PATH_LENGTH];
    int result = parsePath(path, retParent, &index, lastElemName);
    if (index == -1 || retParent == NULL) {
        return -1; // Safety check
    }
    if (result == -1 || retParent[index].isDirectory == 0) {
        return -1; // Invalid path or not directory or directory not found
    }

    DE *temp = loadDir(&retParent[index]);
    // Free the previous cwd
    if (cwd != root) {
        free(cwd);
    }
    cwd = temp; // Update the cwd directory entry

    // Update the cwdString
    char newCwdString[MAX_PATH_LENGTH];
    if (path[0] == '/') {
        // Absolute path
        strncpy(newCwdString, path, MAX_PATH_LENGTH - 1);
        newCwdString[MAX_PATH_LENGTH - 1] = '\0';
    } else {
        // Relative path
        snprintf(newCwdString, MAX_PATH_LENGTH, "%s%s", cwdString, path);
    }

    // Normalize the path
    char *token;
    char *rest = newCwdString;
    char *tokens[MAX_PATH_LENGTH];
    int tokenCount = 0;

    while ((token = strtok_r(rest, "/", &rest))) {
        if (strcmp(token, ".") == 0) {
            continue; // Ignore "."
        } else if (strcmp(token, "..") == 0) {
            if (tokenCount > 0) {
                tokenCount--; // Go back one directory
            }
        } else {
            tokens[tokenCount++] = token; // Add to tokens
        }
    }

    // Build the normalized path
    cwdString[0] = '/';
    cwdString[1] = '\0';
    for (int i = 0; i < tokenCount; i++) {
        strncat(cwdString, tokens[i], MAX_PATH_LENGTH - strlen(cwdString) - 2);
        strncat(cwdString, "/", MAX_PATH_LENGTH - strlen(cwdString) - 1);
    }

    return 0; // Success
}

// Function to get the current working directory
char* getCwd() {
    return cwdString;
}