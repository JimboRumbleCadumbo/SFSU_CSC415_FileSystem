// TODO Rishita : work on fs_setcwd  and fs_getcwd

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs_structs.h" // Include necessary structures and definitions

#define MAX_PATH_LENGTH 1024

// Global variable to store the current working directory
char currentWorkingDirectory[MAX_PATH_LENGTH] = "/";

// Function to validate the path
int validatePath(const char *path) {
    // Check if the path is not NULL and not empty
    if (path == NULL || strlen(path) == 0) {
        return -1; // Invalid path
    }
    // Additional validation can be added here
    return 0; // Valid path
}

// Function to check if the directory exists
int directoryExists(const char *path) {
    // Implement logic to check if the directory exists in the filesystem
    // This is a placeholder implementation
    // In a real implementation, you would traverse the filesystem structure
    // to check if the directory exists
    return 1; // Assuming that the directory exists for now
}

// Function to set the current working directory
int fs_setcwd(const char *path) {
    // Validate the input path
    if (validatePath(path) != 0) {
        fprintf(stderr, "Error: Invalid path\n");
        return -1; // Invalid path
    }

    // Check if the directory exists
    if (!directoryExists(path)) {
        fprintf(stderr, "Error: Directory does not exist\n");
        return -1; // Directory does not exist
    }

    // Update the current working directory
    strncpy(currentWorkingDirectory, path, MAX_PATH_LENGTH - 1);
    currentWorkingDirectory[MAX_PATH_LENGTH - 1] = '\0'; // Ensure null-termination

    return 0; // Success
}

// Function to get the current working directory
const char* fs_getcwd() {
    return currentWorkingDirectory;
}