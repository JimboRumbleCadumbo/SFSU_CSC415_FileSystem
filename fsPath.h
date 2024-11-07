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
* Description:: This file contains the Header for fsPath.c
*
**************************************************************/
#ifndef FSPATH_H
#define FSPATH_H
#include <stdio.h>
#include <string.h>
#include "fsDirUtility.h"
#include "structs.h"    

int parsePath(char *path, DE *retParent, int *index, char *lastElemName);

#endif