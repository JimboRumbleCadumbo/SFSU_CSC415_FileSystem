/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo, riathakker, yyyuquan, Rishita317 
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: structs.h
*
* Description:: all functions related to directories
*
**************************************************************/
#include <time.h>
#include "functions.h"
#include "fsLow.h"

DE * createDirectory(int numEntries, DE *parent) {
    // Allocate memory by determining bytes needed & determining block boundaries
    int bytesNeeded = numEntries * sizeof(DE);
    int blocksNeeded = (bytesNeeded + (vcb->blockSize - 1))/vcb->blockSize;
    int actualBytes = blocksNeeded * vcb->blockSize;
    DE *newDir;
    newDir = (DE *)malloc(actualBytes);
    if (newDir == NULL) {
        printf("Malloc failed");
        return NULL;
    }
    // get a location on the FAT for the file
    int location = allocateBlocks(blocksNeeded);
    int actualEntries = actualBytes / sizeof(DE);
    // Set everything but . and .. entries as unused
    for (int i = 2; i < numEntries; i++) {
        newDir[i].name[0] = '\0'; 
    }
    // Initialize . entry
    strcpy(newDir[0].name, ".");
    newDir[0].location = location;
    newDir[0].size = actualBytes;
    newDir[0].isDirectory = 1;
    time_t current = (time_t)time;
    newDir[0].timeCreated = current;
    newDir[0].timeModified = current;
    // Initialize .. as the parent entry or as itself in root case
    if (parent == NULL) { // Null passed in if creating root directory
        parent = newDir; // Set parent as itself
    }
    strcpy(newDir[1].name, "..");
    newDir[1].location = parent[0].location;
    newDir[1].size = parent[0].size;
    newDir[1].isDirectory = 1;
    newDir[1].timeCreated = parent[0].timeCreated;
    newDir[1].timeModified = parent[0].timeModified;
    if (writeDir(newDir) < 1) {
        printf("Error writing directory");
        return NULL;
    }
    return newDir;
}

int writeDir(DE *dir) {
    int blocks = (dir[0].size + (vcb->blockSize - 1))/vcb->blockSize;
    int blocksWritten = discontinuousWrite(dir->location, (void *)dir);
    return blocksWritten;
}