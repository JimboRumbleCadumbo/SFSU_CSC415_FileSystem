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
* Description:: all the structures here
*
**************************************************************/
#ifndef STRUCTS_H
#define STRUCTS_H
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>	
#include <string.h>	
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

typedef struct VCB {
    int signature; // Signature
    int numBlocks; // number of blocks in the filesystem
    int blockSize; // size of the blocks
    int tableLoc; // location of the first block of the FAT table
    int rootLoc; // location of the root directory
    int freeSpaceLoc; // the first index of free space
} VCB;

typedef struct DE {
    time_t timeCreated; 
    time_t timeModified; 
    int size;
    int isDirectory; 
    int location;
    char name[225];
} DE;

// global VCB variable
extern VCB *vcb;
extern int *fat;
#endif