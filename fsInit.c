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

#include "fsLow.h"
#include "mfs.h"
#include "structs.h"

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	
	int usrSignature = 0x0000AAAA;

	VCB *vcbPoint = malloc(blockSize);

	LBAread(vcbPoint, 1, 0);

	if (vcbPoint == NULL) {
		fprintf(stderr, "Error: LBAread returned NULL\n");
		return -1; // Handle error appropriately
	}
	
	if(vcbPoint->signature != usrSignature){
		vcbPoint->signature = usrSignature;
		vcbPoint->numBlocks = numberOfBlocks;
		vcbPoint->blockSize = blockSize;
		// vcbPoint->tableLoc = ;	
		// vcbPoint->rootLoc = ;
	}
	
	// ------ For Testing Purposes ------ //
	printf("\n\nAFTER Null check\n\n");
	printf("\nsignature: %d\n", vcbPoint->signature);
	printf("\nnumBlocks: %d\n", vcbPoint->numBlocks);
	printf("\nblockSize: %d\n", vcbPoint->blockSize);

	LBAwrite(vcbPoint, 1, 0);

	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}