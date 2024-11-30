/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: fsReadWrite.h
*
* Description:: This file contains Header for fsReadWrite.c
*
**************************************************************/
#ifndef FSREADWRITE_H
#define FSREADWRITE_H
#include <sys/types.h>
#include "structs.h"
#include "fsLow.h"
#include "fsFreeSpaceAllo.h"

int discontinuousWrite(int startingBlock, void *buffer);
int discontinuousRead(int startingBlock, void *buffer); 
int discontinuousPartialWrite(int startingBlock, void *buffer, int numBlocksToWrite);
int discontinuousPartialRead(int startingBlock, void *buffer, int numBlocksToRead);
int moveBlockIndex(int startingBlock, int numBlocksToMove);
int extendChain(int numBlocksToExtend, int startingBlock);
int reduceChain(int numBlocksToReduce, int startingBlock);
int writeFAT();
#endif