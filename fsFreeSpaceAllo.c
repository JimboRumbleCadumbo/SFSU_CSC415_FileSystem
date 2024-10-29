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
#include <structs.h>
#include "fsFreeSpace.c"
#include "fsInit.c"

extern int *fat;
extern VCB vcb;

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