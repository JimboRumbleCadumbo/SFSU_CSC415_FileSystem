/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo, riathakker, yyyuquan, Rishita317 
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: fs_setcwd.c
*
* Description:: This file contains the header for fsCurrentDir.c
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fsPath.h"
#include "structs.h" 

#ifndef FSCURRENTDIR_H
#define FSCURRENTDIR_H

int setCwd(const char *path);
const char* getCwd();

#endif