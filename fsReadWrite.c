/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: fsReadWrite.c
*
* Description:: This file contains wrapper functions to support discontinuous 
* read and write. 
* These functions are essential for handling non-contiguous blocks in the 
* File Allocation Table (FAT), ensuring that data can be read from and written 
* to the filesystem efficiently and correctly.
*
**************************************************************/

#include "fsReadWrite.h"

int discontinuousWrite(int startingBlock, void *buffer) {
    int currentBlock = startingBlock;
    int bytesWritten = 0;
    while (fat[currentBlock] != END_OF_CHAIN) {
        LBAwrite(buffer + bytesWritten, 1, currentBlock);
        bytesWritten += vcb->blockSize;
        currentBlock = fat[currentBlock];
    }
    LBAwrite(buffer + bytesWritten, 1, currentBlock);
    bytesWritten += vcb->blockSize;
    int blocksWritten = (bytesWritten / vcb->blockSize);
    return blocksWritten;
}

int discontinuousRead(int startingBlock, void *buffer) {
    int currentBlock = startingBlock;
    int bytesRead = 0;
    while (fat[currentBlock] != END_OF_CHAIN) {
        LBAread(buffer + bytesRead, 1, currentBlock);
        bytesRead += vcb->blockSize;
        currentBlock = fat[currentBlock];
    }
    int blocksRead = LBAread(buffer + bytesRead, 1, currentBlock);
    return blocksRead;
}

int writeFAT() {
    int numFATBlocks = ((vcb->numBlocks * sizeof(int)) + (vcb->blockSize - 1))/vcb->blockSize;
    int blocksWritten = LBAwrite(fat, numFATBlocks, vcb->tableLoc);
    if (blocksWritten != numFATBlocks) {
        printf("Error writing all blocks of FAT. %d blocks written. \n", blocksWritten);
        return -1;
    }
    return blocksWritten;
}