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

/**
 * int discontinuousWrite(int startingBlock, void *buffer)
 * 
 * Description: By utilizing the LBAwrite function, this function is made to
 * write data one block at a time discontinuouslly, until the last block in the 
 * chain.
 * 
 * @param startingBlock The block address that we wish to start writing from
 * @param buffer The pointer pointing to the start of the block chain.
 * @return The number of blocks WRITTEN into the FAT table
 */
int discontinuousWrite(int startingBlock, void *buffer) {
    int currentBlock = startingBlock;
    int bytesWritten = 0;

    int i = 0;
    while (fat[currentBlock] != END_OF_CHAIN) {
        LBAwrite(buffer + bytesWritten, 1, currentBlock);
        bytesWritten += vcb->blockSize;
        currentBlock = fat[currentBlock];
        i++;
        if(i > vcb->numBlocks){
            printf("\nExceeding vcb numBlocks, EOC not found.\n");
            return -1;
        }
    }

    LBAwrite(buffer + bytesWritten, 1, currentBlock);
    bytesWritten += vcb->blockSize;
    int blocksWritten = (bytesWritten / vcb->blockSize);
    return blocksWritten;
}

/**
 * int discontinuousRead(int startingBlock, void *buffer)
 * 
 * Description: By utilizing the LBAwrite function, this function is made to
 * read data one block at a time discontinuouslly, until the last block in the 
 * chain.
 * 
 * @param startingBlock The block address that we wish to start reading from
 * @param buffer The pointer pointing to the start of the block chain.
 * @return The number of blocks READ from the FAT table
 */
int discontinuousRead(int startingBlock, void *buffer) {
    int currentBlock = startingBlock;
    int bytesRead = 0;

    int i = 0;
    while (fat[currentBlock] != END_OF_CHAIN) {
        LBAread(buffer + bytesRead, 1, currentBlock);
        bytesRead += vcb->blockSize;
        currentBlock = fat[currentBlock];
        i++;
        if(i > vcb->numBlocks){
            printf("\nExceeding vcb numBlocks, EOC not found.\n");
            return -1;
        }
    }

    int blocksRead = LBAread(buffer + bytesRead, 1, currentBlock);
    return blocksRead;
}

/**
 * int discontinuousPartialWrite(int startingBlock, void *buffer, int numBlocksToWrite)
 * 
 * Description: This funtion is similar to discontinuousWrite, with an extra 
 * parameter to specifiy the amount of blocks to write.
 * 
 * @param startingBlock The block address that we wish treado start reading from
 * @param buffer The pointer pointing to the start of the block chain
 * @param numBlocksToWrite The number of blocks to WRITE from the block chain
 * @return The number of blocks READ from the FAT table
 */
int discontinuousPartialWrite(int startingBlock, void *buffer, int numBlocksToWrite) {
    int currentBlock = startingBlock;
    int writeCount = numBlocksToWrite;
    int bytesWritten = 0;
    int writeReturn = 0;

    while (writeCount > 0 && currentBlock != -1) {
        writeReturn = LBAwrite(buffer + bytesWritten, 1, currentBlock);
        if(writeReturn < 0){
            printf("[disconti-PWrite]LBAwrite Error......\n");
            return -1;
        }
        bytesWritten += vcb->blockSize;
        currentBlock = fat[currentBlock];
        writeCount--;
    }

    // Return check for ensuring
    int blocksWritten = (bytesWritten / vcb->blockSize);
    if (blocksWritten != numBlocksToWrite){
        printf("blocksWritten is not equal to numBlocksToWrite......\n");
        return -1;
    }

    return blocksWritten;
}

/**
 * int discontinuousPartialRead(int startingBlock, void *buffer, int numBlocksToRead)
 * 
 * Description: This funtion is similar to discontinuousRead, with an extra 
 * parameter to specifiy the amount of blocks to read.
 * 
 * @param startingBlock The block address that we wish to start reading from
 * @param buffer The pointer pointing to the start of the block chain
 * @param numBlocksToRead The number of blocks to READ from the block chain
 * @return The number of blocks READ from the FAT table
 */
int discontinuousPartialRead(int startingBlock, void *buffer, int numBlocksToRead) {
    int currentBlock = startingBlock;
    int readCount = numBlocksToRead;
    int bytesRead = 0;
    int blocksRead = 0;
    
    while (readCount > 0 && currentBlock != -1){
        blocksRead = LBAread(buffer + bytesRead, 1, currentBlock);
        if(blocksRead < 0){
            printf("[disconti-PRead]LBAread Error......\n");
            return -1;
        }
        bytesRead += vcb->blockSize;
        currentBlock = fat[currentBlock];
        readCount--;
    }

    // Return check for ensuring
    if(blocksRead - numBlocksToRead != 0){
        printf("blocksRead is not equal to numBlocksToRead......\n");
        return -1;
    }

    return blocksRead;
}

/**
 * int moveBlockIndex(int startingBlock, int numBlocksToMove)
 * 
 * Description: Moves the block index by the number of blocks specified
 * 
 * @param startingBlock The block index to start at
 * @param numBlocksToMove The number of blocks to move
 * @return The new block index
 */
int moveBlockIndex(int startingBlock, int numBlocksToMove) {
    if(numBlocksToMove < 0){
        printf("[moveBlockIndex] numBlocksToMove is negative\n");
        return -1;
    }
    
    
    for (int i = 0; i < numBlocksToMove; i++) {
        
        if(startingBlock == END_OF_CHAIN){
            printf("[moveBlockIndex] Reached end of chain\n");
            return -1;
        }
        startingBlock = fat[startingBlock];
    }
    
    return startingBlock;
}

/**
 * int extendChain(int numBlocksToExtend, int startingBlock)
 * 
 * Description: Extends the FAT chain by the number of numBlocksToExtend.
 * 
 * @param numBlocksToExtend The number of blocks to extend from the chain
 * @param startingBlock The head of the chain to start extending from
 * @return 0 for success, -1 for error.
 */
int extendChain(int numBlocksToExtend, int startingBlock){
    int currentBlock = startingBlock;

    while(fat[currentBlock] != END_OF_CHAIN){
        // currentBlock++;
        currentBlock = fat[currentBlock];
    }

    fat[currentBlock] = vcb->freeSpaceLoc;
    printf("[extendChain] Extending chain from block %d", vcb->freeSpaceLoc);
    int result = allocateBlocks(numBlocksToExtend);
    if (result < 0) {
        printf("Error allocating blocks.\n");
        return -1;
    }

    return 0;
}

/**
 * int reduceChain(int numBlocksToReduce, int startingBlock)
 * 
 * Description: Reduce the FAT chain by the number of numBlocksToReduce.
 * 
 * @param numBlocksToReduce The number of blocks to reduce from the chain
 * @param startingBlock The head of the chain to start releasing from
 * @return 0 for success, -1 for error.
 */
int reduceChain(int numBlocksToReduce, int startingBlock){
    int currentBlock = startingBlock;

    while(fat[currentBlock] != END_OF_CHAIN){
        currentBlock++;
        currentBlock = fat[currentBlock];
    }
    
    currentBlock -= numBlocksToReduce;
    fat[currentBlock] = END_OF_CHAIN;

    int release = releaseBlocks(numBlocksToReduce, currentBlock + 1);
    if(release < 0){
        printf("Error releasing blocks.\n");
        return -1;
    }

    return 0;
}

/**
 * int writeFAT() 
 * 
 * Description: This function is for writing the FAT table to the disk.
 * 
 * @return The number blocks written into the FAR table.
 */
int writeFAT() {
    int numFATBlocks = ((vcb->numBlocks * sizeof(int)) + (vcb->blockSize - 1))/vcb->blockSize;
    int blocksWritten = discontinuousWrite(vcb->tableLoc, fat);

    if (blocksWritten != numFATBlocks) {
        printf("Error writing all blocks of FAT. %d blocks written. \n", blocksWritten);
        return -1;
    }
    
    return blocksWritten;
}