/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: fsInitDir.h
*
* Description:: This file contains the Header for fsDirUtility.c
*
**************************************************************/

#include "fsDirUtility.h"

void freeDir (DE *dir) {
    // Selective free: don't free root or current working dir
    if (dir == NULL || dir == root || dir == cwd) {
        return;
    }
    free(dir);
}
DE * loadDir(DE *dir) {
    if (dir == NULL || dir->isDirectory != 1) {
        return NULL; // Invalid argument
    }
    if (dir == root) {
        return root;
    }
    int bytesNeeded;
    int blocksNeeded = (dir->size + (vcb->blockSize - 1)) / vcb->blockSize;
    bytesNeeded = blocksNeeded * vcb->blockSize;
    DE *new = (DE *)malloc(bytesNeeded);
    discontinuousRead(dir->location, new);
    if (new == NULL) {
        printf("Error with reading");
        return NULL;
    }
    return new;
}
int findInDir(DE *parent, char *name) {
    if (parent == NULL || name == NULL) {
        return -2; // sentinel value for invalid input args
    }
    int numEntries = parent[0].size / sizeof(DE);
    for (int i = 0; i < numEntries; i++) {
        // Check valid name
        if (parent[i].name != NULL || strlen(parent[i].name) != 0) {
            if (strcmp(parent[i].name, name) == 0) {
                return i; // Index of where the directory is found
            }
        }
    }
    return -1; // Directory not found
}
int firstUnusedDirEntry(DE *parent) {
    int numEntries = parent[0].size / sizeof(DE);
    for (int i = 0; i < numEntries; i++) {
        if (parent[i].name == NULL) {
            return i;
        }
    }
    return -1; // All entries used
}