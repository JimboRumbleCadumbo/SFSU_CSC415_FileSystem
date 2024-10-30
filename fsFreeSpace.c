/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo, riathakker, yyyuquan, Rishita317 
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: fsFreeSpace.c
*
* Description:: The fsFreeSpace.c file is essential for managing the
  free space in the filesystem. It initializes the FAT, ensuring that 
  blocks are correctly linked and marked, and writes this information to disk.
  This setup is crucial for the efficient allocation and deallocation 
  of blocks in the filesystem.
*
**************************************************************/

// VCB is a global variable 

#include <stdlib.h>
#include <string.h>
#include "functions.h"

#define END_OF_CHAIN 0xFFFFFFFF

// Function to initialize the FAT
int initializeFAT(int blockSize, int numBlocks) {
    // Allocate memory for the FAT
    int totalBlocks = (numBlocks * sizeof(int)) + (blockSize - 1)/blockSize;
    int totalBytes = totalBlocks * blockSize;
    fat = (int *)malloc(totalBytes);
    if (fat == NULL) {
        return -1; // Memory allocation failed
    }

//  // link everything together 0 by itself and then block 1-2, 3-4 etc 
// and thats how the total is FAT i is i+1 
// last block 156 should be null or so we know its the end. // 0 is the VCB

    // Initialize the FAT
    for (int i = 0; i < numBlocks; i++) {
        if (i == 0) {
            fat[i] = -1; // VCB is by itself
        } else if (i < 155) {
            fat[i] = i + 1; // Link the blocks together
        } else {
            fat[i] = i + 1; // Mark the rest as free
        }
    }
    fat[155] = END_OF_CHAIN; // Mark the last block (156) as the end
    fat[numBlocks-1] = END_OF_CHAIN;
    
    // Write the FAT to disk
    if (discontinuousWrite(1, fat) != numBlocks) {
        free(fat);
        return -1; // Write failed
    }
    // Return the starting block number of the FAT
    return 1; // FAT starts at block 1
}