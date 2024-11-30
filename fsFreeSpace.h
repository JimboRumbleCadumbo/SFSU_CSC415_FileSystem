/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: fsFreeSpace.h
*
* Description:: Header for fsFreeSpace.c
*
**************************************************************/
#ifndef FSFREESPACE_H
#define FSFREESPACE_H
#include <stdlib.h>
#include <string.h>

#include "structs.h"
#include "fsReadWrite.h"

int initializeFAT(int blockSize, int numBlocks);
#endif