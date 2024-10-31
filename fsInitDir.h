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
* Description:: This file contains the Header for fsInit.c
*
**************************************************************/
#ifndef FSINITDIR_H
#define FSINITDIR_H
#include <time.h>
#include "fsFreeSpaceAllo.h"
#include "fsReadWrite.h"
#include "structs.h"    
#include "fsLow.h"

DE * createDirectory(int numEntries, DE *parent);
int writeDir(DE *dir);
#endif