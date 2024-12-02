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
#ifndef FSDIRUTILITY_H
#define FSDIRUTILITY_H
#include <stdio.h>
#include <string.h>

#include "fsReadWrite.h"
#include "structs.h"

void freeDir(DE *dir);
DE *loadDir(DE *dir);
int findInDir(DE *parent, char *name);
int firstUnusedDirEntry(DE *parent);
int isDirEmpty(DE *parent);
int removeDirectoryEntry(DE *dir, int indexToRemove);

#endif