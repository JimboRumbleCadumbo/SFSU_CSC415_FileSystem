/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo, riathakker, yyyuquan, Rishita317 
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: fs_getcwd.c
*
* Description:: This file contains the implementation of the 
* fs_getcwd function, which retrieves the current working directory 
* in the filesystem. This function is crucial for implementing 
* other filesystem operations such as print working directory (pwd).
**************************************************************/

#include <stdio.h>
#include <string.h>

#define MAX_PATH_LENGTH 1024

// Global variable to store the current working directory
extern char currentWorkingDirectory[MAX_PATH_LENGTH];

// Function to get the current working directory
const char* fs_getcwd(char *buffer, size_t size) {
    // Check if the buffer is NULL or size is 0
    if (buffer == NULL || size == 0) {
        return NULL; // Invalid buffer or size
    }

    // Check if the size is sufficient to hold the current working directory
    if (size < strlen(currentWorkingDirectory) + 1) {
        return NULL; // Buffer size is too small
    }

    // Copy the current working directory to the buffer
    strncpy(buffer, currentWorkingDirectory, size - 1);
    buffer[size - 1] = '\0'; // Ensure null-termination

    return buffer; // Return the buffer containing the current working directory
}