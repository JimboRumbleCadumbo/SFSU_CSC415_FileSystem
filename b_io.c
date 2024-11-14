/**************************************************************
* Class::  CSC-415-03 Fall 2024
* Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi 	
* Student IDs:: 923313947, 922593253, 920390312, 922249026
* GitHub-Name:: JimboRumbleCadumbo
* Group-Name:: Bytes Busters
* Project:: Basic File System
*
* File:: b_io.c
*
* Description:: Basic File System - Key File I/O Operations
*
**************************************************************/
// Rishita to work on read and seek

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"

#include "fsLow.h"
#include <stdlib.h>

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
{
    char *buf;    // holds the open file buffer
    int index;    // holds the current position in the buffer
    int buflen;   // holds how many valid bytes are in the buffer
    int fileDescriptor; // file descriptor
    int filePointer;    // current position in the file
    int fileSize;       // size of the file
    int blockSize;      // size of a block
} b_fcb;

b_fcb fcbArray[MAXFCBS];

int startup = 0;    // Indicates that this has not been initialized

// Method to initialize our file system
void b_init()
{
    // init fcbArray to all free
    for (int i = 0; i < MAXFCBS; i++)
    {
        fcbArray[i].buf = NULL; // indicates a free fcbArray
    }

    startup = 1;
}

// Method to get a free FCB element
b_io_fd b_getFCB()
{
    for (int i = 0; i < MAXFCBS; i++)
    {
        if (fcbArray[i].buf == NULL)
        {
            return i;        // Not thread safe (But do not worry about it for this assignment)
        }
    }
    return (-1);  // all in use
}

// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open(char *filename, int flags)
{
    b_io_fd returnFd;

    //*** TODO ***:  Modify to save or set any information needed
    //
    //

    if (startup == 0) b_init();  // Initialize our system

    returnFd = b_getFCB();                // get our own file descriptor
                                          // check for error - all used FCB's

    return (returnFd);                        // all set
}

// Interface to seek function    
int b_seek(b_io_fd fd, off_t offset, int whence)
{
    if (startup == 0) b_init();  // Initialize our system

    // check that fd is between 0 and (MAXFCBS-1)
    if ((fd < 0) || (fd >= MAXFCBS))
    {
        return (-1);                     // invalid file descriptor
    }

    return (0); // Change this
}

// Interface to write function    
int b_write(b_io_fd fd, char *buffer, int count)
{
    if (startup == 0) b_init();  // Initialize our system

    // check that fd is between 0 and (MAXFCBS-1)
    if ((fd < 0) || (fd >= MAXFCBS))
    {
        return (-1);                     // invalid file descriptor
    }

    return (0); // Change this
}

// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+

int b_read(b_io_fd fd, char *buffer, int count)
{
    if (startup == 0) b_init();  // Initialize our system

    // Check that fd is between 0 and (MAXFCBS-1)
    if ((fd < 0) || (fd >= MAXFCBS))
    {
        return -1;  // Invalid file descriptor
    }

    // Get the file control block
    b_fcb *fcb = &fcbArray[fd];

    // Check if the file is open
    if (fcb->fileDescriptor == -1)
    {
        return -1;  // File not open
    }

    // Calculate the number of bytes to read
    int bytesToRead = count;
    if (fcb->filePointer + bytesToRead > fcb->fileSize)
    {
        bytesToRead = fcb->fileSize - fcb->filePointer;  // Adjust bytes to read if it exceeds file size
    }

    // Read data from the file into the buffer
    int bytesRead = LBAread(buffer, bytesToRead, fcb->filePointer / fcb->blockSize);
    if (bytesRead < 0)
    {
        return -1;  // Error reading file
    }

    // Update the file pointer
    fcb->filePointer += bytesRead;

    // Return the number of bytes read
    return bytesRead;
}

// Interface to Close the file    
int b_close(b_io_fd fd)
{
    // Check that fd is between 0 and (MAXFCBS-1)
    if ((fd < 0) || (fd >= MAXFCBS))
    {
        return -1;  // Invalid file descriptor
    }

    // Get the file control block
    b_fcb *fcb = &fcbArray[fd];

    // Check if the file is open
    if (fcb->fileDescriptor == -1)
    {
        return -1;  // File not open
    }

    // Free the buffer
    if (fcb->buf != NULL)
    {
        free(fcb->buf);
        fcb->buf = NULL;
    }

    // Reset the file control block
    fcb->fileDescriptor = -1;
    fcb->filePointer = 0;
    fcb->fileSize = 0;
    fcb->blockSize = 0;

    return 0;  // Success
}