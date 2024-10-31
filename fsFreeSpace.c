/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: fsFreeSpace.c
*
* Description:: The file is essential for out filesystem project. 
* The fsFreeSpace.c file is important for managing the
* free space in the filesystem. It initializes the FAT, ensuring that 
* blocks are correctly linked and marked, and writes this information to disk.
* This setup is crucial for the efficient allocation and deallocation 
* of blocks in the filesystem.
*
**************************************************************/

#include "fsFreeSpace.h"

// Function to initialize the FAT
int initializeFAT(int blockSize, int numBlocks) {
    // Allocate memory for the FAT
    int totalBlocks = ((numBlocks * sizeof(int)) + (blockSize - 1))/blockSize;
    totalBlocks;
    int totalBytes = totalBlocks * blockSize;
    fat = (int *)malloc(totalBytes);
    if (fat == NULL) {
        return -1; // Memory allocation failed
    }
    printf("Total blocks: %d\n", totalBlocks);

//  // link everything together 0 by itself and then block 1-2, 3-4 etc 
// and thats how the total is FAT i is i+1 
// last block 156 should be null or so we know its the end. // 0 is the VCB

    // Initialize the FAT
    for (int i = 0; i < numBlocks; i++) {
        if (i == 0) {
            fat[i] = -1; // VCB is by itself
        } else if (i == totalBlocks || i == numBlocks - 1) {
            // Mark the end of chain for the FAT table and the free space
            fat[i] = END_OF_CHAIN; 
        }
        else {
            fat[i] = i + 1; // Link everything else together.
        }
    }
    vcb->freeSpaceLoc = totalBlocks + 1;
    
    // Write the FAT to disk
    if (discontinuousWrite(1, fat) != totalBlocks) {
        printf("Writing failed");
        free(fat);
        fat = NULL;
        printf("Freed FAT\n");
        return -1; // Write failed
    }
    // Return the starting block number of the FAT
    return 1; // FAT starts at block 1
}