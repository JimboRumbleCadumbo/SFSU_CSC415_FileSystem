/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo, riathakker, yyyuquan, Rishita317 
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: fsallo.c
*
* Description:: Allocation for the FAT
*
**************************************************************/

#include <stdio.h>
#include <stdlib.h> 
#include "fsFreeSpace.c"

// Function to allocate the FAT
int allocateBlock(){
    int FAT[TOTAL_BLOCKS]; // Initialize the FAT
    for (int i = 0; i < TOTAL_BLOCKS; i++) {
        if (FAT[i] == 0) { // Check if the block is free
            FAT[i] = 1; // Mark the block as allocated
            return i; // Return the index of the allocated block
        }
    }
    return -1;
}