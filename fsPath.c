/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: fsInitDir.h
*
* Description:: This file contains the Header for fsPath.c
*
**************************************************************/
#include "fsPath.h"

/**
 * int parsePath(char *path, DE *retParent, int *index, char *lastElemName)
 * 
 * Description: Takes in a string that represents a path to a file or directory
 * in the logical file system directory tree. Tokenizes the path and parses it to 
 * useful information to the file system that will return the appropriate directory
 * entry.
 * 
 * Arguments retParent, index, and lastElemName are passed in by reference by the caller
 * as values to be returned.
 * 
 * @param path String that contains the path the caller specified
 * @param retParent Return value: the given file/directory's parent directory
 * @param index Return value: the index in the parent directory at which the current file/dir 
 *              is located
 * @param lastElemName Return value: the string containing the name of the last file/dir
 *                     specified in the path: relevant to the caller's interests
 */
int parsePath(char *path, DE **retParent, int *index, char *lastElemName) {
    // Check for valid path 
    printf("Beginning parse path. Path is %s\n", path);
    if (path == NULL || strlen(path) == 0) {
        printf("Invalid path specified.\n");
        return -1;
    }
    // Check for relative vs absolute path
    DE *start;
    if (path[0] == '/') {
        // Absolute path starts at root directory
        printf("Starting at root directory.\n");
        start = root;
    } else {
        // Relative path starts at current working directory
        printf("Starting at current working directory.\n");
        printf("Current working directory: %s\n", cwdString);
        start = cwd;
    }
    DE *parent;
    parent = start;
    // Prepare to tokenize path string
    char *token1;
    char *token2;
    char *saveptr;
    token1 = strtok_r(path, "/", &saveptr);
    if (token1 == NULL) {
        // Path is just '/'
        // printf("The path is just '/'.\n");
        *retParent = parent;
        if (index != NULL) {
            *index = 0;
        }
        return 0;
    }
    while (1) {
        token2 = strtok_r(NULL, "/", &saveptr);
        int idx = findInDir(parent, token1);
        if (token2 == NULL) { // End of path 
            *retParent = parent;
            if (index != NULL) {
                *index = idx;
            }
            printf("Setting lastElemName to: %s\n", token1);
            if (*retParent == root) {
                printf("Setting retparent to root\n");
            }
            strncpy(lastElemName, token1, MAX_NAME_LENGTH);
            lastElemName[MAX_NAME_LENGTH - 1] = '\0';  // Null-terminate at the last possible position
            printf("Returning index %d, lastElemName %s, retParent %p vs. root %p\n", *index, lastElemName, retParent, root);
            return 0;
        } else { // token1 is not the last token
            // Need to confirm that token1 exists & is a directory
            if (idx == -1) {
                return -1; // Invalid path
            }
            if (parent[idx].isDirectory != 1) {
                return -1; // Not a directory
            }
            printf("Moving forward.\n");
            DE *newParent = loadDir(&parent[idx]);
            freeDir(parent);
            parent = newParent;
            token1 = token2;
        }
    }
}