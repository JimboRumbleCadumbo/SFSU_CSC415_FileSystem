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
#include "structs.h"

DE * createDirectory(int numEntries, DE *parent) {
    // Allocate memory by determining bytes needed & determining block boundaries
    int bytesNeeded = numEntries * sizeof(DE);
    int blocksNeeded = (bytesNeeded + (vcb.blockSize - 1))/vcb.blockSize;
    int actualBytes = blocksNeeded * vcb.blockSize;
    DE *new = malloc(actualBytes);
    if (new == NULL) {
        printf("Malloc failed");
        return NULL;
    }

}