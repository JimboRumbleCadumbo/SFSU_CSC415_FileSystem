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
 * Description: Allocate a number of blocks, chain them together, write a
 * END_OF_CHAIN at the last block of chain, then update the FAT table.
 *
 * @param numBlocks The number of blocks that need to be allocated
 * @return The starting block of the chain
 */
int allocateBlocks(int numBlocks)
{
    int currentBlock = vcb->freeSpaceLoc;
    int startBlock = currentBlock;

    if (vcb->numBlocks < numBlocks)
    {
        printf("[[Critical]] Insufficient space available in the storage.\n");
        return END_OF_CHAIN;
    }

    if (vcb->freeBlocks == 0)
    {
        printf("[[Critical]] Insufficient space available in the storage.\n");
        return END_OF_CHAIN;
    }

    for (int i = 0; i < numBlocks; i++)
    {
        int nextFreeBlock = fat[currentBlock];

        if (i == numBlocks - 1)
        {
            fat[currentBlock] = END_OF_CHAIN;
            vcb->freeSpaceLoc = nextFreeBlock;
            vcb->freeBlocks--;
        }
        else
        {
            if (currentBlock == END_OF_CHAIN)
            {
                printf("[[Critical]] Internal error has occured.\n");
                return -1;
            }
            fat[currentBlock] = nextFreeBlock;
            currentBlock = nextFreeBlock;
            vcb->freeBlocks--;
        }
    }

    if (writeFAT() < 0)
    {
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
int releaseBlocks(int numToRelease, int startingBlock)
{

    if (startingBlock < 0 || startingBlock >= vcb->numBlocks)
    {
        return -1;
    }

    // Reset the bytes on disk.
    char buf[vcb->blockSize];
    memset(buf, 0, vcb->blockSize);
    discontinuousWrite(startingBlock, buf);

    // Find end of free space chain.
    int freeSpaceEnd;
    int currentBlock = vcb->freeSpaceLoc;

    while (fat[currentBlock] != END_OF_CHAIN)
    {
        currentBlock = fat[currentBlock];
    }
    freeSpaceEnd = currentBlock;

    // Set the end of the free space chain to point to the starting block
    // Since a chain with a valid end is already passed through,
    // the free space chain extends to add the blocks.
    fat[freeSpaceEnd] = startingBlock;

    if (writeFAT() < 0)
    {
        return -1;
    }

    return vcb->freeSpaceLoc;
}
