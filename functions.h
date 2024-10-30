/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: functions.h
*
* Description:: A combination of all the functions in fsReadWrite,
* fsFreeSpace, fsInitDir.c
*
**************************************************************/

#include "structs.h"
#define END_OF_CHAIN 0xFFFFFFFF

int allocateBlocks(int numBlocks);
int releaseBlocks(int numToRelease, int startingBlock);
int discontinuousWrite(int startingBlock, void *buffer);
int discontinuousRead(int startingBlock, void *buffer);
int initializeFAT();
DE * createDirectory(int numEntries, DE *parent);
int writeDir(DE *dir);

