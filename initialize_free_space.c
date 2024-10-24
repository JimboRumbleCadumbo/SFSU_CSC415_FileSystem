#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512
#define TOTAL_BLOCKS 19531
#define FREE_SPACE_BLOCKS 5

// Function to initialize the free space map
int initializeFreeSpace() {
    // Allocate memory for the free space map
    unsigned char *freeSpaceMap = (unsigned char *)malloc(FREE_SPACE_BLOCKS * BLOCK_SIZE);
    if (freeSpaceMap == NULL) {
        return -1; // Memory allocation failed
    }

    // Initialize the free space map
    memset(freeSpaceMap, 0xFF, FREE_SPACE_BLOCKS * BLOCK_SIZE); // Set all bits to 1 (used)
    freeSpaceMap[0] = 0xFC; // First 6 bits used (VCB and free space map itself)

    // Write the free space map to disk
    if (LBAwrite(freeSpaceMap, FREE_SPACE_BLOCKS, 1) != FREE_SPACE_BLOCKS) {
        free(freeSpaceMap);
        return -1; // Write failed
    }

    // Free the allocated memory
    free(freeSpaceMap);

    // Return the starting block number of the free space map
    return 1; // Free space map starts at block 1
}