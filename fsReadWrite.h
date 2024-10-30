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
* Description:: Header for fsReadWrite.c
*
**************************************************************/

#include "structs.h"
#include "fsLow.h"

int discontinuousWrite(int startingBlock, void *buffer);
int discontinuousRead(int startingBlock, void *buffer); 