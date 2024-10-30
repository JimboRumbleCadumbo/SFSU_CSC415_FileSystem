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
	// ------ For Testing Purposes ------ //
	printf("\n\nAFTER Null check\n\n");

	// If the signature doesn't match or does not exist, format the VCB
	if(vcb->signature != usrSignature){
		printf("Assigning signature\n");
		vcb->signature = usrSignature;
		printf("Assigning numBlocks\n");
		vcb->numBlocks = numberOfBlocks;
		printf("Assigning blockSize\n");
		vcb->blockSize = blockSize;
		printf("Initializing FAT\n");
		vcb->tableLoc = initializeFAT(blockSize, numberOfBlocks);	
		printf("Initializing Root Directory\n");
		root = createDirectory(50, NULL);

		// ------ For Testing Purposes ------ //
		printf("\nsignature: %X\n", vcb->signature);
		printf("\nnumBlocks: %d\n", vcb->numBlocks);
		printf("\nblockSize: %d\n", vcb->blockSize);
		printf("\ntableLoc: %d\n", vcb->tableLoc);
		printf("\nrootLoc: %d\n", vcb->rootLoc);
		// ---------------------------------- //

		LBAwrite(vcb, 1, 0);
		printf("\n\nDisk Initialized... \n\n");
	}
	else{ // Retrive data that is already in disk

		printf("\n\nThe disk was already initialized...Reading table & rootDir from disk... \n\n");

		// ** Need to grab the location of the root directory here

		printf("\n\nRead Complete...... \n\n");
	}

	return 0;
	}
	
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