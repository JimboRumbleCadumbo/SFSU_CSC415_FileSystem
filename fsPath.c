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
int parsePath(char *path, DE *retParent, int *index, char *lastElemName) {
    // Check for valid path 
    if (path == NULL || strlen(path) == 0) {
        return -1;
    }
    // Check for relative vs absolute path
    DE *start;
    if (path[0] == '/') {
        // Absolute path starts at root directory
        start = root;
    } else {
        // Relative path starts at current working directory
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
        retParent = parent;
        lastElemName = NULL;
        if (index != NULL) {
            *index = 0;
        }
        return 0;
    }
    while (1) {
        token2 = strtok_r(NULL, "/", &saveptr);
        // TODO: Create helper function FindInDirectory
        int idx = FindInDirectory(parent, token1);
        if (token2 == NULL) { // End of path 
            retParent = parent;
            if (index != NULL) {
                *index = idx;
            }
            lastElemName = token1;
            return 0;
        } else { // token1 is not the last token
            // Need to confirm that token1 exists & is a directory
            if (idx == -1) {
                return -1; // Invalid path
            }
            if (parent[idx].isDirectory != 1) {
                return -1; // Not a directory
            }
            DE *newParent = loadDir(&parent[idx]);
            // TODO: Implement FreeDirectory selective free
            FreeDirectory(parent);
            parent = newParent;
            token1 = token2;
        }
    }
}