/**************************************************************
 * Class::  CSC-415-03 Fall 2024
 * Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi
 * Student IDs:: 923313947, 922593253, 920390312, 922249026
 * GitHub-Name:: JimboRumbleCadumbo
 * Group-Name:: Bytes Busters
 * Project:: Basic File System
 *
 * File:: fsInit.c
 *
 * Description:: Main driver for file system assignment.
 *
 * This file is where you will start and initialize your system
 *
 **************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "b_io.h"
#include "mfs.h"
#include "fsLow.h"
#include "fsFreeSpace.h"
#include "fsInitDir.h"

// Initialize global variable locally for use.
VCB *vcb = NULL;
int *fat = NULL;
DE *root = NULL;
DE *cwd = NULL;
char cwdString[MAX_PATH_LENGTH] = "";

/**
 * int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
 *
 * Description: This function initializes the File System with the needed
 * structure and information of the disk. Check the disk signature first, if it
 * does not match or the signature is not there, write the essential metadata
 * into the disk; if the signature matches, read metadata off from the disk into
 * memory.
 *
 * @param numberOfBlocks The number of blocks (aka storage) that we wish to give
 * to the file system.
 * @param blockSize The size of each block.
 * @return 0 if success, -1 if failed.
 */
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	int usrSignature = 0x1234ABCD;

	vcb = malloc(blockSize);

	if (vcb == NULL)
	{
		printf("[[Critical]] VCB block allocation failed\n");
		return -1;
	}

	if (LBAread(vcb, 1, 0) < 0)
	{
		printf("[[Critical]] Error occured while initializing system\n");
		return -1;
	}

	// If the signature doesn't match or does not exist, format the VCB
	if (vcb->signature != usrSignature)
	{
		vcb->signature = usrSignature;
		vcb->numBlocks = numberOfBlocks;
		vcb->blockSize = blockSize;
		vcb->tableLoc = initializeFAT(blockSize, numberOfBlocks);
		root = createDirectory(50, NULL);

		if (LBAwrite(vcb, 1, 0) < 0)
		{
			printf("[[Critical]] Error occured while formatting\n");
			return -1;
		}
	}
	else
	{
		// Retrive data that is already in disk

		int numBlocksInFat = (vcb->numBlocks * sizeof(int)) 
							 + (vcb->blockSize - 1) / vcb->blockSize;
		fat = malloc(numBlocksInFat * vcb->blockSize);

		if (fat == NULL)
		{
			printf("[[Critical]] FAT table allocation failed\n");
			return -1;
		}

		if (LBAread(fat, numBlocksInFat, vcb->tableLoc) < 0)
		{
			printf("[[Critical]] Error occured while retrieving data from disk\n");
			return -1;
		}

		int numBytesInRoot = (sizeof(DE) * 50);
		int numBlocksInRoot = (numBytesInRoot + (vcb->blockSize - 1)) / vcb->blockSize;
		root = malloc(numBlocksInRoot * vcb->blockSize);

		if (root == NULL)
		{
			printf("[[Critical]] Root directory allocation failed\n");
			return -1;
		}

		if (LBAread(root, numBlocksInRoot, vcb->rootLoc) < 0)
		{
			printf("[[Critical]] Error occured while fetching root directory data\n");
			return -1;
		}

		printf("\nRead Complete...... \n\n");
	}
	// Initialize current working directory as the root directory
	int numBytesInDir = (sizeof(DE) * 50);
	int numBlocksInDir = (numBytesInDir + (vcb->blockSize - 1)) / vcb->blockSize;
	cwd = malloc(numBlocksInDir * vcb->blockSize);

	if (cwd == NULL)
	{
		printf("[[Critical]] Current working directory allocation failed\n");
		return -1;
	}

	// Set current working directory to root directory at init
	cwd = root;
	char *rootPath = "/";
	strcpy(cwdString, rootPath);
	cwdString[strlen(cwdString)] = '\0';
	b_init();
	return 0;
}

/**
 * void exitFileSystem()
 *
 * Description: Free the pointers that were used in this file.
 */
void exitFileSystem()
{
	if (LBAwrite(vcb, 1, 0) < 0)
	{
		printf("[[Critical]] Error occured while saving data\n");
	}
	free(vcb);
	vcb = NULL;
	free(fat);
	fat = NULL;
	if (cwd != root)
	{
		free(cwd);
		cwd = NULL;
	}
	free(root);
	root = NULL;
	printf("System exiting\n");
}