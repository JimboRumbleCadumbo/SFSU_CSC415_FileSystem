/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo, riathakker, yyyuquan, Rishita317 
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
#include "structs.h"
#include "fsFreeSpace.c"

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	
	int usrSignature = 0x1234ABCD;

	VCB *vcbPoint = malloc(blockSize);

	LBAread(vcbPoint, 1, 0);

	if (vcbPoint == NULL) {
		fprintf(stderr, "Error: LBAread returned NULL\n");
		return -1; // Handle error appropriate
	}
	// ------ For Testing Purposes ------ //
	printf("\n\nAFTER Null check\n\n");

	// If the signature doesn't match or does not exist, format the VCB
	if(vcbPoint->signature != usrSignature){
		vcbPoint->signature = usrSignature;
		vcbPoint->numBlocks = numberOfBlocks;
		vcbPoint->blockSize = blockSize;
		vcbPoint->tableLoc = initializeFAT();	
		// vcbPoint->rootLoc = ;

		// ------ For Testing Purposes ------ //
		printf("\nsignature: %d\n", vcbPoint->signature);
		printf("\nnumBlocks: %d\n", vcbPoint->numBlocks);
		printf("\nblockSize: %d\n", vcbPoint->blockSize);
		printf("\ntableLoc: %d\n", vcbPoint->tableLoc);
		// printf("\nrootLoc: %d\n", vcbPoint->rootLoc);
		// ---------------------------------- //

		LBAwrite(vcbPoint, 1, 0);
		printf("\n\nDisk Initialized... \n\n");
	}
	else{ // Retrive data that is already in disk

		printf("\n\nThe disk was already initialized...Reading table & rootDir from disk... \n\n");

		// ** Need to grab the location of the root directory here

		printf("\n\nRead Complete...... \n\n");
	}

	return 0;
	}
	
void exitFileSystem ()
	{
		free(fat);
		printf ("System exiting\n");
	}