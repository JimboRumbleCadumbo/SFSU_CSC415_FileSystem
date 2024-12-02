/**************************************************************
 * Class::  CSC-415-03 Fall 2024
 * Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi
 * Student IDs:: 923313947, 922593253, 920390312, 922249026
 * GitHub-Name:: JimboRumbleCadumbo
 * Group-Name:: Bytes Busters
 * Project:: Basic File System
 *
 * File:: fsFreeSpaceAllo.h
 *
 * Description:: This file contains the Header for fsFreeSpaceAllo.c
 *
 **************************************************************/
#ifndef FSFREESPACEALLO_H
#define FSFREESPACEALLO_H
#include <stdio.h>
#include <stdlib.h>

#include "structs.h"
#include "fsReadWrite.h"

#define END_OF_CHAIN 0xFFFFFFFF

int allocateBlocks(int numBlocks);
int releaseBlocks(int numToRelease, int startingBlock);
#endif