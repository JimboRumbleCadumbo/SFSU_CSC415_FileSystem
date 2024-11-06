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

#include "mfs.h"
#include "fsLow.h"
#include "fsFreeSpace.h"
#include "fsInitDir.h"

// Initialize global variable locally for use.
VCB *vcb = NULL;
int *fat = NULL;
DE *root = NULL;
DE *cwd = NULL;

/**
 * int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
 * 
 * Description: This function initializes the File System with the needed 
 * structure and information of the disk. Check the disk signature first, if it
 * does not match or the signature is not there, write the essential metadata
 * into the disk; if the signature matches, read metadata off from the disk into 
 * memory.
 * 
 */
int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	
	
	int usrSignature = 0x1234ABCD;

	vcb = malloc(blockSize);

	LBAread(vcb, 1, 0);

	if (vcb == NULL) {
		fprintf(stderr, "Error: LBAread returned NULL\n");
		return -1; // Handle error appropriate
	}

	// If the signature doesn't match or does not exist, format the VCB
	if(vcb->signature != usrSignature){
		vcb->signature = usrSignature;
		vcb->numBlocks = numberOfBlocks;
		vcb->blockSize = blockSize;
		vcb->tableLoc = initializeFAT(blockSize, numberOfBlocks);	
		root = createDirectory(50, NULL);

		// ------ For Testing Purposes ------ //
		// printf("\nsignature: %X\n", vcb->signature);
		// printf("\nnumBlocks: %d\n", vcb->numBlocks);
		// printf("\nblockSize: %d\n", vcb->blockSize);
		// printf("\ntableLoc: %d\n", vcb->tableLoc);
		// printf("\nrootLoc: %d\n", vcb->rootLoc);
		// ---------------------------------- //

		LBAwrite(vcb, 1, 0);
		printf("\n\nDisk Initialized... \n\n");
	}
	else{ 
		// Retrive data that is already in disk
		printf("\n\nThe disk was already initialized...Reading table & rootDir from disk... \n\n");

		int numBlocksInFat = (vcb->numBlocks * sizeof(int)) + (vcb->blockSize - 1) / vcb->blockSize;
		fat = malloc(numBlocksInFat * vcb->blockSize);

		if (fat == NULL) {
			printf("Malloc failed\n");
		}

		LBAread(fat, numBlocksInFat, vcb->tableLoc);
		// printf("Sanity check for fat in memory %d \n %d \n %d\n", fat[0], fat[1], fat[152]);
		
		int numBytesInRoot = (sizeof(DE) * 50);
		int numBlocksInRoot = (numBytesInRoot + (vcb->blockSize - 1))/vcb->blockSize;
		root = malloc(numBlocksInRoot * vcb->blockSize);
		cwd = malloc(numBlocksInRoot * vcb->blockSize);

		if (root == NULL) {
			printf("Malloc for root failed\n");
		}
		if (cwd == NULL) {
			printf("Malloc for root failed\n");
		}

		LBAread(root, numBlocksInRoot, vcb->rootLoc);

		// Set current working directory to root directory at init
		cwd = root;

		// printf("Sanity check for root %s\n", root[0].name);
		printf("\n\nRead Complete...... \n\n");
	}

	return 0;
	}
	
/**
 * void exitFileSystem()
 * 
 * Description: Free the pointers that were used in this file.
 */
void exitFileSystem()
	{
		free(vcb);
		vcb = NULL;
		free(fat);
		fat = NULL;
		free(root);
		root = NULL;
		printf ("System exiting\n");
	}