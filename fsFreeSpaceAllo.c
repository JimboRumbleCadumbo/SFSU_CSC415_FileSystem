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
* Description:: Set of routine that help allocating free blocks for
* the user to use.
*
**************************************************************/

#include "fsFreeSpace.h"

// Allocate a chain of blocks
int allocateBlocks(int numBlocks) {

    printf("Allocating %d blocks...\n", numBlocks);

    int currentBlock = vcb->freeSpaceLoc;
    int startBlock = currentBlock;

    printf("vcb->tableLoc = %d, vcb->numBlocks = %d\n", vcb->freeSpaceLoc, vcb->numBlocks);


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
            fat[currentBlock] = END_OF_CHAIN;
            vcb->freeSpaceLoc = nextFreeBlock;
        } else {
            fat[currentBlock] = nextFreeBlock;
            currentBlock = nextFreeBlock;
        }
    }

    //vcb->numBlocks -= numBlocks;
    printf("Total free blocks after allocation: %d\n", vcb->numBlocks);

    printf("Done. Start block of the allocated chain: %d\n", startBlock);
    return startBlock;
}

// Release a chain of blocks
int releaseBlocks(int numToRelease, int startingBlock) {
    printf("Releasing %d blocks starting from block %d...\n", numToRelease, startingBlock);

    int currentLoc = startingBlock;

    for (int i = 0; i < numToRelease; i++) {
        int nextBlock = fat[currentLoc];
        printf("Freeing block %d\n", currentLoc);

        if (i == numToRelease - 1) {
            fat[currentLoc] = vcb->freeSpaceLoc;
            printf("Linking block %d to the start of free space at %d\n", currentLoc, vcb->tableLoc);
        }
        
        currentLoc = nextBlock;
    }

    vcb->freeSpaceLoc = startingBlock;
    printf("New start of free space: %d\n", vcb->freeSpaceLoc);

    return vcb->freeSpaceLoc;
}