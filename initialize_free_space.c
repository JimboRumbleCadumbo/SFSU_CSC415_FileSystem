// By: Rishita Meharishi

#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512
#define TOTAL_BLOCKS 19531
#define FAT_ENTRIES (TOTAL_BLOCKS)
#define FAT_BLOCKS ((FAT_ENTRIES * sizeof(int) + BLOCK_SIZE - 1) / BLOCK_SIZE) // Calculate number of blocks needed for FAT

// Function to initialize the FAT
int initializeFAT() {
    // Allocate memory for the FAT
    int *fat = (int *)malloc(FAT_BLOCKS * BLOCK_SIZE);
    if (fat == NULL) {
        return -1; // Memory allocation failed
    }

    // Initialize the FAT
    for (int i = 0; i < FAT_ENTRIES; i++) {
        if (i < 6) {
            fat[i] = -1; // Mark first 6 entries as used (VCB and FAT itself)
        } else {
            fat[i] = 0; // Mark the rest as free
        }
    }

    // Write the FAT to disk
    if (LBAwrite(fat, FAT_BLOCKS, 1) != FAT_BLOCKS) {
        free(fat);
        return -1; // Write failed
    }

    // Free the allocated memory
    free(fat);

    // Return the starting block number of the FAT
    return 1; // FAT starts at block 1
}