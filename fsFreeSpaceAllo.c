/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo, riathakker, yyyuquan, Rishita317 
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: fsallo.c
*
* Description:: Allocation for the FAT
*
**************************************************************/

#include <stdio.h>
#include <stdlib.h> 
#include "functions.h"


// Allocate a chain of blocks
int allocateBlocks(int numBlocks) {

    printf("Allocating %d blocks...\n", numBlocks);

    int currentBlock = vcb.tableLoc;
    int startBlock = currentBlock;

    printf("vcb.tableLoc = %d, vcb.numBlocks = %d\n", vcb.tableLoc, vcb.numBlocks);


    if (vcb.numBlocks < numBlocks) {
        printf("Not enough free space. Required: %d, Available: %d\n", numBlocks, vcb.numBlocks);
        return END_OF_CHAIN;
    }

    for (int i = 0; i < numBlocks; i++) {
        int nextFreeBlock = fat[currentBlock];
        
        if (i == numBlocks - 1) {
            fat[currentBlock] = END_OF_CHAIN;
            vcb.tableLoc = nextFreeBlock;
        } else {
            fat[currentBlock] = nextFreeBlock;
            currentBlock = nextFreeBlock;
        }
    }

    vcb.numBlocks -= numBlocks;
    printf("Total free blocks after allocation: %d\n", vcb.numBlocks);

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
            fat[currentLoc] = vcb.tableLoc;
            printf("Linking block %d to the start of free space at %d\n", currentLoc, vcb.tableLoc);
        }
        
        currentLoc = nextBlock;
    }

    vcb.tableLoc = startingBlock;
    printf("New start of free space: %d\n", vcb.tableLoc);

    return vcb.tableLoc;
}