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
* Description:: Stuffs related to initializing the freespace
*
**************************************************************/

// VCB is a global variable 

#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512
#define TOTAL_BLOCKS 19531
#define FAT_ENTRIES (TOTAL_BLOCKS)
// Calculate number of blocks needed for FAT
#define FAT_BLOCKS ((FAT_ENTRIES * sizeof(int) + BLOCK_SIZE - 1) / BLOCK_SIZE) 
#define END_OF_CHAIN 0xFFFFFFFF

extern int *fat;

// Function to initialize the FAT
int initializeFAT() {
    // Allocate memory for the FAT
    fat = (int *)malloc(FAT_BLOCKS * BLOCK_SIZE);
    if (fat == NULL) {
        return -1; // Memory allocation failed
    }

//  // link everything together 0 by itself and then block 1-2, 3-4 etc 
// and thats how the total is FAT i is i+1 
// last block 156 should be null or so we know its the end. // 0 is the VCB

    // Initialize the FAT
    for (int i = 0; i < FAT_ENTRIES; i++) {
        if (i == 0) {
            fat[i] = -1; // VCB is by itself
        } else if (i < 155) {
            fat[i] = i + 1; // Link the blocks together
        } else {
            fat[i] = i + 1; // Mark the rest as free
        }
    }
    fat[155] = END_OF_CHAIN; // Mark the last block (156) as the end
    fat[FAT_ENTRIES-1] = END_OF_CHAIN;
    
    // Write the FAT to disk
    if (discontinuousWrite(1, fat) != FAT_BLOCKS) {
        free(fat);
        return -1; // Write failed
    }
    // Return the starting block number of the FAT
    return 1; // FAT starts at block 1
}