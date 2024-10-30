/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo, riathakker, yyyuquan, Rishita317 
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: fsReadWrite.c
*
* Description:: wrapper functions to support discontinuous 
* read and write 
*
**************************************************************/
#include "functions.h"
#include "fsLow.h"

int discontinuousWrite(int startingBlock, void *buffer) {
    printf("Starting block is %d\n", startingBlock);
    int currentBlock = startingBlock;
    int bytesWritten = 0;
    while (fat[currentBlock] != END_OF_CHAIN) {
        printf("Writing block %d to disk.\n", currentBlock);
        LBAwrite(buffer + bytesWritten, 1, currentBlock);
        bytesWritten += vcb->blockSize;
        currentBlock = fat[currentBlock];
    }
    printf("Writing block %d to disk.\n", currentBlock);
    LBAwrite(buffer + bytesWritten, 1, currentBlock);
    bytesWritten += vcb->blockSize;
    int blocksWritten = (bytesWritten / vcb->blockSize);
    printf("Returning %d\n", blocksWritten);
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