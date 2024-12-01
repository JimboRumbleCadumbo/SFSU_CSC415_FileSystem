/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: fsInitDir.c
*
* Description:: Helper funtions to help dealing with actions relating
* to directory entries.
*
**************************************************************/

#include "fsDirUtility.h"

/**
 * void freeDir (DE *dir) 
 * 
 * Description: Free target directory entry. (Note: doesn't free root & current 
 * working directory)
 * 
 * @param dir The DE structure to be freed
 */
void freeDir (DE *dir) {
    // Selective free: don't free root or current working dir
    if (dir == NULL || dir->location == root->location || dir->location == cwd->location) {
        return;
    }
    free(dir);
    dir = NULL;
}

DE * loadDir(DE *dir) {
    if (dir == NULL ) {
        printf("Error: Invalid argument\n");
        return NULL; // Invalid argument
    }
    if(dir->isDirectory != 1){
        printf("Error: Not a directory\n");
        return NULL; // Not a directory
    }
    if (dir->location == root->location) {
        return root;
    }
    if (dir->location == cwd->location) {
        return cwd;
    }
    int bytesNeeded;
    int blocksNeeded = (dir->size + (vcb->blockSize - 1)) / vcb->blockSize;
    bytesNeeded = blocksNeeded * vcb->blockSize;
    DE *new = (DE *)malloc(bytesNeeded);
    discontinuousRead(dir->location, new);

    if (new == NULL) {
        printf("[[Critical]] Failed to read directory\n");
        return NULL;
    }
    
    return new;
}

/**
 * int findInDir(DE *parent, char *name) 
 * 
 * Description: Find the directory that has the required name in the paramater 
 * from the parent directory.
 * 
 * @param parent The directory entry that we are searaching from
 * @param name The name of the DE that we wish to find in parent
 * @return The index of the DE if found, -1 if not found
 */
int findInDir(DE *parent, char *name) {
    if (parent == NULL || name == NULL) {
        return -2; // sentinel value for invalid input args
    }

    int numEntries = parent[0].size / sizeof(DE);
    for (int i = 0; i < numEntries; i++) {
        // Check valid name        
        if (parent[i].name != NULL || strlen(parent[i].name) != 0) {
            if (strcmp(parent[i].name, name) == 0) {
                return i; // Index of where the directory is found
            }
        }
    }

    return -1; // Directory not found
}

/**
 * int firstUnusedDirEntry(DE *parent)
 * 
 * Description: Find the first unused directory entry.
 * 
 * @param parent The directory entry that we wish to look from
 * @return the index of the first unused entry, -1 if all entries are occupied.
 */
int firstUnusedDirEntry(DE *parent) {
    int numEntries = parent[0].size / sizeof(DE);

    for (int i = 0; i < numEntries; i++) {
        if (parent[i].name[0] == '\0') {
            return i;
        }
    }

    return -1; // All entries used
}

/**
 * int isDirEmpty(DE *parent)
 * 
 * Description: Check if the directory entry is empty.
 * 
 * @param parent The directory that we wish to check
 * @return 1 if the directory is empty, 0 if not.
 */
int isDirEmpty(DE *parent) {
    int numEntries = parent[0].size / sizeof(DE);

    for (int i = 2; i < numEntries; i++) {
        if (parent[i].name[0] != '\0') {
            return 0;
        }
    }
    
    return 1;
}

/**
 * int removeDirectoryEntry(DE *dir)
 * 
 * Description: Shift entries in a directory if it is removed
 * 
 * @param dir The directory to be shifted
 * @param indexToRemove The index of the entry to shift
 * @return 0 if success, -1 if failure
 */
int removeDirectoryEntry(DE *dir, int indexToRemove) {
    if (dir == NULL || indexToRemove < 0 || indexToRemove >= ENTRIES_IN_DIR) {
        return -1;
    }

    // Shift entries after the removed one
    for (int i = indexToRemove; i < ENTRIES_IN_DIR - 1; i++) {
        dir[i] = dir[i + 1];
    }

    // Clear the last entry
    memset(&dir[ENTRIES_IN_DIR - 1], 0, sizeof(DE));
    return 0;
}
