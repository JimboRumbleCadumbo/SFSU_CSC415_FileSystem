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
// Rishita to work on read, seek ,close and write

#include "b_io.h"

#include "fsLow.h"
#include <stdlib.h>

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
{
    char *buf;          // holds the open file buffer
    int index;          // holds the current position in the buffer
    int buflen;         // holds how many valid bytes are in the buffer
    int fileDescriptor; // file descriptor
    int filePointer;    // current position in the file
    int fileSize;       // size of the file
    int blockSize;      // size of a block
} b_fcb;

b_fcb fcbArray[MAXFCBS];

int startup = 0; // Indicates that this has not been initialized

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
            return i; // Not thread safe (But do not worry about it for this assignment)
        }
    }
    return (-1); // all in use
}


b_io_fd b_open(char *filename, int flags)
{

    if (startup == 0)
    {
        b_init(); // Initialize our system
    }

    printf("Starting open function. \n");
    b_io_fd returnFd;

    //*** TODO ***:  Modify to save or set any information needed
    //
    //
    DE *retParent;
    int index = 0;
    char lastElemName[MAX_NAME_LENGTH];
    printf("Parsing path. \n");
    int result = parsePath(filename, &retParent, &index, lastElemName);
    if (result < 0)
    {
        printf("Invalid path. \n");
        return -1;
    }
    printf("Successfully parsed path. \n");
    printf("Allocating memory for FCB. \n");
    b_fcb fcb;
    time_t now = time(NULL);
    // Create the file if it doesn't exist
    if (flags & O_CREAT)
    {
        printf("Create flag specified. \n");
        if (index < 0)
        {
            printf("File does not already exist. \n");
            index = firstUnusedDirEntry(retParent);
            if (index < 0)
            {
                printf("Unused DE not found in parent. \n");
                return -1; // No more unused DEs in the parent
            }
            printf("Found unused DE #%d in parent directory. \n", index);
            printf("Populating DE in parent directory. \n");
            strcpy(retParent[index].name, lastElemName);
            retParent[index].size = 0;
            retParent[index].location = 0;
            retParent[index].timeCreated = now;
            retParent[index].isDirectory = 0;
            fcb.filePointer = 0;
            fcb.fileSize = 0;
            fcb.index = 0;
            fcb.buflen = 0;
        }
    }
    // Truncate existing file
    if (flags & O_TRUNC)
    {
        printf("Truncate flag specified. \n");
        if (index < 0)
        {
            printf("File does not exist.\n");
            return -1;
        }
        int numBytesToRelease = retParent[index].size;
        int blocksToRelease = (numBytesToRelease + (vcb->blockSize - 1)) / vcb->blockSize;
        printf("Releasing blocks back to free space. \n");
        if (releaseBlocks(blocksToRelease, blocksToRelease) < 0)
        {
            printf("Error releasing blocks. \n");
            return -1;
        }
        retParent[index].size = 0;
        retParent[index].location = 0;
        fcb.filePointer = 0;
        fcb.fileSize = 0;
        fcb.index = 0;
        fcb.buflen = 0;
    }

    printf("Getting file control block. \n");
    returnFd = b_getFCB(); // get our own file descriptor
                           // check for error - all used FCB's
    printf("FCB is %d\n", returnFd);
    if (returnFd < 0)
    {
        printf("All available FCBs are used.\n");
        return -1;
    }
    printf("Modifying parent directory. \n");
    fcb.fileDescriptor = returnFd;
    retParent[index].timeModified = now;
    fcb.blockSize = vcb->blockSize;
    char *buf = malloc(B_CHUNK_SIZE);
    if (buf == NULL)
    {
        printf("Error allocating memory to buffer.\n");
        return -1;
    }
    fcb.buf = buf;
    printf("Writing parent directory to disk.\n");
    if (writeDir(retParent) < 1)
    {
        printf("Error writing parent directory.\n");
        free(buf);
        return -1;
    }
    fcbArray[returnFd] = fcb;
    return (returnFd); // all set
}

// Interface to seek function
// also make sure that seek aslo loads the buffer

int b_seek(b_io_fd fd, off_t offset, int whence)
{
    if (startup == 0)
        b_init(); // Initialize our system

    // Check that fd is between 0 and (MAXFCBS-1)
    if ((fd < 0) || (fd >= MAXFCBS))
    {
        return -1; // Invalid file descriptor
    }

    // Get the file control block
    b_fcb *fcb = &fcbArray[fd];

    // Check if the file is open
    if (fcb->fileDescriptor == -1)
    {
        return -1; // File not open
    }

    // Calculate the new file pointer position
    off_t newPointer;
    switch (whence)
    {
    case SEEK_SET:
        newPointer = offset;
        break;
    case SEEK_CUR:
        newPointer = fcb->filePointer + offset;
        break;
    case SEEK_END:
        newPointer = fcb->fileSize + offset;
        break;
    default:
        return -1; // Invalid whence value
    }

    // Check for invalid new pointer position
    if (newPointer < 0 || newPointer > fcb->fileSize)
    {
        return -1; // Invalid new pointer position
    }

    // Set the new file pointer position
    fcb->filePointer = newPointer;

    // Load the buffer with the appropriate block
    int blockNumber = fcb->filePointer / fcb->blockSize;
    int blockOffset = fcb->filePointer % fcb->blockSize;

    fcb->buflen = LBAread(fcb->buf, 1, blockNumber);
    if (fcb->buflen < 0)
    {
        return -1; // Error reading file
    }
    fcb->index = blockOffset;

    return newPointer; // Return the new file pointer position
}

// Interface to write function
int b_write(b_io_fd fd, char *buffer, int count)
{
    if (startup == 0)
        b_init(); // Initialize the system

    // Validate the file descriptor
    if (fd < 0 || fd >= MAXFCBS || fcbArray[fd].buf == NULL)
    {
        printf("Invalid file descriptor or buffer.\n");
        return -1; // Invalid fd
    }

    b_fcb *fcb = &fcbArray[fd];

    // Check if the file is writable
    if (fcb->fileDescriptor < 0)
    {
        printf("File is not open or writable.\n");
        return -1;
    }

    int bytesWritten = 0;

    while (bytesWritten < count)
    {
        // Space available in the buffer
        int spaceInBuffer = fcb->blockSize - fcb->index;

        // Bytes to write into the buffer
        int bytesToBuffer = (count - bytesWritten < spaceInBuffer) ? count - bytesWritten : spaceInBuffer;

        // Copy data to the buffer
        memcpy(fcb->buf + fcb->index, buffer + bytesWritten, bytesToBuffer);
        fcb->index += bytesToBuffer;
        bytesWritten += bytesToBuffer;

        // Flush buffer to disk if full
        if (fcb->index == fcb->blockSize)
        {
            printf("Flushing buffer to disk...\n");
            int blocksWritten = discontinuousWrite(fcb->fileDescriptor, fcb->buf);
            if (blocksWritten <= 0)
            {
                printf("Error during discontinuous write.\n");
                return -1; // Disk write error
            }
            fcb->index = 0; // Reset buffer index
        }
    }

    // Flush any remaining data in the buffer to disk
    if (fcb->index > 0)
    {
        printf("Writing remaining data to disk...\n");
        int blocksWritten = discontinuousWrite(fcb->fileDescriptor, fcb->buf);
        if (blocksWritten <= 0)
        {
            printf("Error writing remaining data to disk.\n");
            return -1;
        }
        fcb->index = 0;
    }

    // Update file metadata
    fcb->filePointer += bytesWritten;
    if (fcb->filePointer > fcb->fileSize)
    {
        fcb->fileSize = fcb->filePointer; // Update file size if file grows
    }

    printf("Write complete. Bytes written: %d\n", bytesWritten);
    return bytesWritten;
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
    if (startup == 0)
        b_init(); // Initialize our system

    // Check that fd is between 0 and (MAXFCBS-1)
    if ((fd < 0) || (fd >= MAXFCBS))
    {
        return -1; // Invalid file descriptor
    }

    // Get the file control block
    b_fcb *fcb = &fcbArray[fd];

    // Check if the file is open
    if (fcb->fileDescriptor == -1)
    {
        return -1; // File not open
    }

    // Calculate the number of bytes to read
    int bytesToRead = count;
    if (fcb->filePointer + bytesToRead > fcb->fileSize)
    {
        bytesToRead = fcb->fileSize - fcb->filePointer; // Adjust bytes to read if it exceeds file size
    }

    int totalBytesRead = 0;
    while (bytesToRead > 0)
    {
        // Check if buffer needs to be refilled
        if (fcb->index >= fcb->buflen)
        {
            // Calculate the block number to read from
            int blockNumber = fcb->filePointer / fcb->blockSize;
            int blockOffset = fcb->filePointer % fcb->blockSize;

            // Read the block into the buffer
            fcb->buflen = LBAread(fcb->buf, 1, blockNumber);
            if (fcb->buflen < 0)
            {
                return -1; // Error reading file
            }
            fcb->index = blockOffset;
        }

        // Calculate the number of bytes to copy from the buffer
        int bytesFromBuffer = fcb->buflen - fcb->index;
        if (bytesFromBuffer > bytesToRead)
        {
            bytesFromBuffer = bytesToRead;
        }

        // Copy data from the buffer to the user's buffer
        memcpy(buffer + totalBytesRead, fcb->buf + fcb->index, bytesFromBuffer);

        // Update pointers and counters
        fcb->index += bytesFromBuffer;
        fcb->filePointer += bytesFromBuffer;
        totalBytesRead += bytesFromBuffer;
        bytesToRead -= bytesFromBuffer;
    }

    // Return the number of bytes read
    return totalBytesRead;
}

// Interface to Close the file
int b_close(b_io_fd fd)
{
    // Check that fd is between 0 and (MAXFCBS-1)
    printf("Starting close. \n");
    if ((fd < 0) || (fd >= MAXFCBS))
    {
        printf("Invalid file descriptor passed. \n");
        return -1; // Invalid file descriptor
    }

    // Get the file control block
    printf("Getting the FCB. \n");
    b_fcb *fcb = &fcbArray[fd];

    // Check if the file is open
    if (fcb->fileDescriptor == -1)
    {
        printf("File is not open. \n");
        return -1; // File not open
    }

    // Free the buffer
    if (fcb->buf != NULL)
    {
        printf("Freeing the buffer. \n");
        free(fcb->buf);
        fcb->buf = NULL;
    }

    // Reset the file control block
    printf("Resetting FCB. \n");
    fcb->fileDescriptor = -1;
    fcb->filePointer = 0;
    fcb->fileSize = 0;
    fcb->blockSize = 0;

    return 0; // Success
}

// Main function to test the file system
// This function tests the basic file operations of your file system. 
//It opens a file named "example.txt" for writing, writes the string 
//"Hello, World!" to it, and then closes the file. It also checks for errors 
//during the open and write operations and prints appropriate messages.

int TestOpenWrite()
{
    b_io_fd fd = b_open("example.txt", O_WRONLY | O_CREAT);
    if (fd >= 0)
    {
        char data[] = "Hello, World!";
        int bytesWritten = b_write(fd, data, sizeof(data) - 1);
        if (bytesWritten < 0)
        {
            printf("Error writing to file.\n");
        }
        else
        {
            printf("Successfully written %d bytes.\n", bytesWritten);
        }
        b_close(fd);
    }
    else
    {
        printf("Error opening file.\n");
    }

    return 0;
}

