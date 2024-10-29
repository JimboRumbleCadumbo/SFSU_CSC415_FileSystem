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
#include "fsLow.h"
#include "structs.h"
#include "fsFreeSpace.c"

int discontinuousWrite(int startingBlock, void *buffer) {
    int currentBlock = startingBlock;
    int bytesWritten = 0;
    while (fat[currentBlock] != 0xFFFFFFF) {
        LBAWrite(buffer + bytesWritten, 1, currentBlock);
        bytesWritten += vcb.blockSize;
        currentBlock = fat[currentBlock];
    }
    int blocksWritten = LBAWrite(buffer + bytesWritten, 1, currentBlock);
    return blocksWritten;
}

int discontinuousRead(int startingBlock, void *buffer) {
    int currentBlock = startingBlock;
    int bytesRead = 0;
    while (fat[currentBlock] != 0xFFFFFFFF) {
        LBARead(buffer + bytesRead, 1, currentBlock);
        bytesRead += vcb.blockSize;
        currentBlock = fat[currentBlock];
    }
    int blocksRead = LBARead(buffer + bytesRead, 1, currentBlock);
    return blocksRead;
}