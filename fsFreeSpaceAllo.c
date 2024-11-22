/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: fsFreeSpaceAllo.c
*
* Description:: This file is part of the File System project and contains
* routines for managing free space.
* 
* Why:: Efficiently managing free space is crucial for the filesystem's 
* performance and integrity. These routines ensure that blocks can be
* dynamically allocated and deallocated as needed.
**************************************************************/

#include "fsFreeSpace.h"

/**
 * int allocateBlocks(int numBlocks)
 * 
 * Description: Allocate a chain of blocks, chain them together, write a
 * END_OF_CHAIN at the last block of chain, then update the FAT table.
 * 
 * @param numBlocks The number of blocks that need to be allocated
 * @return The starting block of the chain
 */
int allocateBlocks(int numBlocks) {

    printf("Allocating %d blocks...\n", numBlocks);

    int currentBlock = vcb->freeSpaceLoc;
    int startBlock = currentBlock;

    printf("vcb->freeSpaceLoc = %d, vcb->numBlocks = %d\n", vcb->freeSpaceLoc, vcb->numBlocks);


    if (vcb->numBlocks < numBlocks) {
        printf("Not enough free space. Required: %d, Available: %d\n", numBlocks, vcb->numBlocks);
        return END_OF_CHAIN;
    }

    if (fat[vcb->freeSpaceLoc] == END_OF_CHAIN) {
        printf("No more disc space.\n");
        return END_OF_CHAIN;
    }

    for (int i = 0; i < numBlocks; i++) {
        int nextFreeBlock = fat[currentBlock];
        
        if (i == numBlocks - 1) {
            printf("Setting end of chain to %d\n", currentBlock);
            fat[currentBlock] = END_OF_CHAIN;
            vcb->freeSpaceLoc = nextFreeBlock;
        } 
        else {
            fat[currentBlock] = nextFreeBlock;
            currentBlock = nextFreeBlock;
        }
    }

    printf("Total free blocks after allocation: %d\n", vcb->numBlocks);

    printf("Done. Start block of the allocated chain: %d\n", startBlock);

    if (writeFAT() < 0) {
        printf("Error writing FAT\n");
        return -1;
    } 

    return startBlock;
}

/**
 * int releaseBlocks(int numToRelease, int startingBlock)
 * 
 * Description: Releasing a number of blocks, or delinking, from the chain of 
 * blocks.
 * 
 * @param numToRelease Number of blocks to be released
 * @param startingBlock The block location to start releasing from
 * @return The first free space location in the VCB
 */
int releaseBlocks(int numToRelease, int startingBlock) {
    printf("Releasing %d blocks starting from block %d...\n", numToRelease, startingBlock);

    int currentLoc = startingBlock;

     if (startingBlock < 0 || startingBlock >= vcb->numBlocks) {
        printf("Error: Invalid starting block %d\n", startingBlock);
        return -1;
    }

    for (int i = 0; i < numToRelease; i++) {
        if (currentLoc <= 0 || currentLoc >= vcb->numBlocks) {
            // Avoid freeing past numBlocks or the VCB
            printf("Invalid block number specified.\n");
            return -1;
        }
        int nextBlock = fat[currentLoc];
        printf("Freeing block %d\n", currentLoc);

        fat[currentLoc] = vcb->freeSpaceLoc;
        vcb->freeSpaceLoc = currentLoc;
        
        currentLoc = nextBlock;
    }

    vcb->freeSpaceLoc = startingBlock;
    printf("New start of free space: %d\n", vcb->freeSpaceLoc);

    if (writeFAT() < 0) {
        printf("Error writing FAT\n");
        return -1;
    } 
    
    return vcb->freeSpaceLoc;
}
