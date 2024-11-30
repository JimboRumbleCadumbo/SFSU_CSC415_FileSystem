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
 * Description::What this files does and how it contributes to our File System project.
 * This file (b_io.c) implements the basic file I/O operations for our
 * filesystem project. It provides functions to open, read, write,
 * seek, and close files. The file control block (FCB) structure is
 * used to manage open files and their associated buffers. The key
 * functions in this file include:
 *
 * - b_init(): Initializes the file system by setting up the FCB array.
 * - b_getFCB(): Retrieves a free FCB element.
 * - b_open(): In our open function we are opening a file
 *   and also make sure that the flags below work properly.
 * O_CREAT   - Creates a new file is it does not exist (ignored if the file does exist)
 *O_TRUNC  - Set the file length to 0 (truncates all data)
 *O_APPEND   - Sets the file position to the end of the file
 *(Same as doing a seek 0 from SEEK_END)
 *O_RDONLY   - File can only do read/seek operations
 *O_WRONLY  - File can only do write/seek operations
 *O_RDWR   - File can be read or written to
 *   necessary, and returns a file descriptor.
 * - b_read(): Reads data from an open file into a buffer.
 * - b_write(): Writes data from a buffer to an open file.
 * - b_seek(): Changes the file pointer position for an open file.
 * - b_close(): Closes an open file and frees associated resources.
 *
 * These functions work together to provide a basic interface for
 * file operations, allowing users to interact with the filesystem
 * by opening, reading, writing, seeking, and closing files. The
 * implementation ensures that data is buffered efficiently and
 * handles various file operations correctly.
 *
 **************************************************************/
// essential imports
#include "b_io.h"

#include "fsLow.h"
#include <stdlib.h>

#define MAXFCBS 20
#define B_CHUNK_SIZE vcb->blockSize

typedef struct b_fcb
{
    char *buf;          // holds the open file buffer
    int index;          // holds the current position in the buffer
    int fileLocation;   // holds the file's first block location on the disk
    int buflen;         // holds how many valid bytes are in the buffer
    int fileDescriptor; // file descriptor
    int filePointer;    // current position in the file
    int fileSize;       // size of the file
    int blockSize;      // size of a block
    int accessFlags;    // flags for O_RDONLY, O_WRONLY, O_RDWR
    int isDirty;        // holder for dirty buffer. 1 if dirty, 0 if not
    int deIndex;        // index of the directory entry
    int currentBlk;     // current block for b_read
    DE *directoryEntry; // pointer to the directory entry
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

/**
 * TODO: Make sure all the variables are initialized properly for each flag cases
 */
b_io_fd b_open(char *filename, int flags)
{
    printf("\n[In b_open]\n");
    b_io_fd returnFd;

    DE *retParent;
    int index = 0;
    char lastElemName[MAX_NAME_LENGTH];
    printf("Parsing path. \n");
    int result = parsePath(filename, &retParent, &index, lastElemName);
    if (result < 0)
    {
        printf("Invalid path. \n");
        freeDir(retParent);
        return -1;
    }
    printf("Successfully parsed path. \n");
    printf("Allocating memory for FCB. \n");
    b_fcb fcb;
    time_t now = time(NULL);
    // Create the file if it doesn't exist
    if (flags & O_CREAT)
    {
        printf("[Create flag specified] \n");
        if (index < 0)
        {
            printf("File does not already exist. \n");
            index = firstUnusedDirEntry(retParent);
            if (index < 0)
            {
                printf("Unused DE not found in parent. \n");
                freeDir(retParent);
                return -1; // No more unused DEs in the parent
            }
            printf("Found unused DE #%d in parent directory. \n", index);
            printf("Populating DE in parent directory. \n");
            strcpy(retParent[index].name, lastElemName);
            retParent[index].size = 0;
            retParent[index].location = 0;
            retParent[index].timeCreated = now;
            fcb.fileLocation = retParent[index].location;
            fcb.filePointer = 0;
            fcb.fileSize = 0;
            fcb.index = 0;
            fcb.buflen = 0;
            fcb.directoryEntry = retParent;
            fcb.deIndex = index;
        }
    }
    // Truncate existing file
    if (flags & O_TRUNC)
    {
        printf("[Truncate flag specified] \n");
        if (index < 0)
        {
            printf("File does not exist.\n");
            freeDir(retParent);
            return -1;
        }
        int numBytesToRelease = retParent[index].size;
        int blocksToRelease = (numBytesToRelease + (vcb->blockSize - 1)) / vcb->blockSize;
        // printf("Releasing blocks back to free space. \n");
        // if (releaseBlocks(blocksToRelease, blocksToRelease) < 0)
        // {
        //     printf("Error releasing blocks. \n");
        //     freeDir(retParent);
        //     return -1;
        // }

        retParent[index].isDirectory = 0;
        retParent[index].size = 0;
        retParent[index].location = 0;
        fcb.fileLocation = retParent[index].location;
        fcb.filePointer = 0;
        fcb.fileSize = 0;
        fcb.index = 0;
        fcb.buflen = 0;
    }
    // if (flags & O_RDONLY){
    //     printf("[Read ONLY flag specified] \n");
    //     if (index < 0)
    //     {
    //         printf("File does not exist.\n");
    //         freeDir(retParent);
    //         return -1;
    //     }

    //     fcb.fileLocation = retParent[index].location;
    //     fcb.filePointer = 0;
    //     fcb.fileSize = retParent[index].size;
    //     fcb.index = 0;
    //     fcb.buflen = 0;
    // }

    // Handle O_RDONLY flag
    //  File can only do read/seek operations
    if (flags & O_RDONLY)
    {
        printf("[Read ONLY flag specified] \n");
        if (index < 0)
        {
            printf("File does not exist.\n");
            freeDir(retParent);
            return -1;
        }

        fcb.fileLocation = retParent[index].location;
        fcb.filePointer = 0;
        fcb.fileSize = retParent[index].size;
        fcb.index = 0;
        fcb.buflen = 0;
    }

    // Handle O_WRONLY flag
    //  File can only do write/seek operations
    if (flags & O_WRONLY)
    {
        printf("[Write ONLY flag specified] \n");
        if (index < 0)
        {
            printf("File does not exist.\n");
            freeDir(retParent);
            return -1;
        }

        fcb.fileLocation = retParent[index].location;
        fcb.filePointer = 0;
        fcb.fileSize = retParent[index].size;
        fcb.index = 0;
        fcb.buflen = 0;
    }

    // Handle O_RDWR flag
    // File can be read or written to
    if (flags & O_RDWR)
    {
        printf("[Read/Write flag specified] \n");
        if (index < 0)
        {
            printf("File does not exist.\n");
            freeDir(retParent);
            return -1;
        }

        fcb.fileLocation = retParent[index].location;
        fcb.filePointer = 0;
        fcb.fileSize = retParent[index].size;
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
        freeDir(retParent);
        return -1;
    }
    printf("Modifying parent directory. \n");
    fcb.fileDescriptor = returnFd;
    fcb.accessFlags = flags;
    fcb.blockSize = vcb->blockSize;
    char *buf = malloc(B_CHUNK_SIZE);
    memset(buf, 0, B_CHUNK_SIZE);

    if (buf == NULL)
    {
        printf("Error allocating memory to buffer.\n");
        freeDir(retParent);
        return -1;
    }
    fcb.buf = buf;
    printf("Writing parent directory to disk.\n");
    if (writeDir(retParent) < 1)
    {
        printf("Error writing parent directory.\n");
        free(buf);
        freeDir(retParent);
        return -1;
    }
    fcbArray[returnFd] = fcb;
    if (flags & O_APPEND)
    {
        printf("Append flag specified.\n");
        b_seek(returnFd, 0, SEEK_END);
    }
    freeDir(retParent);

    printf("\n[End b_open]\n");

    return (returnFd); // all set
}

// Interface to seek function
// also make sure that seek aslo loads the buffer

/**
 * int b_seek(b_io_fd fd, off_t offset, int whence)
 *
 * TODO:: If block dirty, write it to disk
 */
int b_seek(b_io_fd fd, off_t offset, int whence)
{
    printf("\n[In b_seek]\n");
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
    off_t newPointer = 0;
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

    printf("\n[End b_seek]\n");

    return newPointer; // Return the new file pointer position
}

// Interface to write function
/**
 * int b_write(b_io_fd fd, char *buffer, int count)
 *
 * TODO: Add flag conditions
 */
int b_write(b_io_fd fd, char *buffer, int count)
{
    printf("\n[In b_write]\n");
    // Validate the file descriptor
    if (fd < 0 || fd >= MAXFCBS || fcbArray[fd].buf == NULL)
    {
        printf("Invalid file descriptor or buffer.\n");
        return -1; // Invalid fd
    }

    b_fcb *fcb = &fcbArray[fd];

    // Check if the file is read-only
    if (fcb->accessFlags & O_RDONLY)
    {
        printf("Error: File is read-only. Cannot write to it.\n");
        return -1;
    }

    // Check if the file is writable
    if (fcb->fileDescriptor < 0)
    {
        printf("File is not open or writable.\n");
        return -1;
    }

    // TODO: Allocate blocks on disk for the file

    if (fcb->fileSize == 0)
    {
        printf("[Write] File size is 0, allocate a block for it\n");
        fcb->fileLocation = allocateBlocks(1); // Allocate a block for the file;
        (fcb->directoryEntry + fcb->deIndex)->location = fcb->fileLocation;
    }

    // P0.5:
    // If count is smaller than the remaining bytes in the current block, write
    // and return.
    if (count < B_CHUNK_SIZE - (fcb->filePointer % B_CHUNK_SIZE))
    {
        printf("[P0.5] Supplement bytes in current block...\n");
        memcpy(fcb->buf + (fcb->filePointer % B_CHUNK_SIZE), buffer, count);
        fcb->filePointer += count;
        if (fcb->fileSize < fcb->filePointer)
        {
            fcb->fileSize = fcb->filePointer;
        }
        fcb->isDirty = 1;
        return count;
    }

    // start writing
    int p1 = 0, p2 = 0;  // No need p3 since p3 = count - p1 - p2
    int targetBlock = 0; //  Holder for the moveBlockIndex result
    int excessBytes = count - (fcb->fileSize - fcb->filePointer);

    // see if current position needs to extend the chain for the incoming count
    if (excessBytes > 0)
    {
        printf("Excess bytes: %d, extending %d blocks...\n", excessBytes,
               (excessBytes + B_CHUNK_SIZE - 1) / B_CHUNK_SIZE);
        extendChain((excessBytes + B_CHUNK_SIZE - 1) / B_CHUNK_SIZE, fcb->fileLocation);
    }

    // p1
    targetBlock = moveBlockIndex(fcb->fileLocation, fcb->filePointer / B_CHUNK_SIZE);
    printf("[p1]targetBlock:%d, %d blocks moved\n", targetBlock, fcb->filePointer / B_CHUNK_SIZE);

    int existingBytes = fcb->filePointer % B_CHUNK_SIZE;
    p1 = B_CHUNK_SIZE - existingBytes;
    memcpy(fcb->buf + existingBytes, buffer, p1);
    fcb->filePointer += p1;
    if (fcb->fileSize < fcb->filePointer)
    {
        fcb->fileSize = fcb->filePointer;
    }
    count -= p1;

    int p1Write = discontinuousPartialWrite(targetBlock, fcb->buf, 1);
    if (p1Write == -1)
    {
        printf("[p1] disc-Write Failed\n");
        return -1;
    }
    targetBlock = moveBlockIndex(targetBlock, 1);
    // p2
    int p2NeededBlocks = count / B_CHUNK_SIZE;
    if (p2NeededBlocks != 0)
    {
        printf("p2 needed blocks: %d\n", p2NeededBlocks);
        p2 = p2NeededBlocks * B_CHUNK_SIZE;
        fcb->filePointer += p2;
        if (fcb->fileSize < fcb->filePointer)
        {
            fcb->fileSize = fcb->filePointer;
        }
        count -= p2;

        int p2Write = discontinuousPartialWrite(targetBlock, buffer + p1, p2NeededBlocks);
        if (p2Write == -1)
        {
            printf("[p2] disc-Write Failed\n");
            return -1;
        }
    }

    // p3
    printf("[p3]targetBlock:%d\n", targetBlock);
    int p3Read = discontinuousPartialRead(targetBlock, fcb->buf, 1);
    if (p3Read == -1)
    {
        printf("[p3] disc-Read Failed\n");
        return -1;
    }
    memcpy(fcb->buf, buffer + p1 + p2, count);
    fcb->filePointer += count;
    if (fcb->fileSize < fcb->filePointer)
    {
        fcb->fileSize = fcb->filePointer;
    }
    fcb->isDirty = 1;

    printf("Write complete. Bytes written: %d\n", count + p1 + p2);
    time_t now = time(NULL);
    fcb->directoryEntry->timeModified = now;

    printf("\n[End b_write]\n");

    return count + p1 + p2;
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

/**
 * TODO: make sure all values in fcb struct are read properly
 */
int b_read(b_io_fd fd, char *buffer, int count)
{
    printf("\n[In b_read]\n");
    // Check that fd is between 0 and (MAXFCBS-1)
    if ((fd < 0) || (fd >= MAXFCBS))
    {
        return -1; // Invalid file descriptor
    }

    // Get the file control block
    b_fcb *fcb = &fcbArray[fd];

    // Check if the file is write-only
    if (fcb->accessFlags & O_WRONLY)
    {
        printf("Error: File is write-only. Cannot read from it.\n");
        return -1;
    }

    // Check if the file is open
    if (fcb->fileDescriptor == -1)
    {
        printf("Invalid file descriptor.\n");
        return -1; // File not open
    }

    int blocksRead;
    int bytesRead;
    int bytesReturned;
    int part1, part2, part3;
    int numBlocksToCopy;
    int remainingBytesInMyBuffer;

    // Calculate bytes available in the buffer
    printf("Bytes requested: %d\n", count);
    remainingBytesInMyBuffer = fcb->buflen - fcb->index;
    printf("Remaining bytes in my buffer: %d\n", remainingBytesInMyBuffer);
    // Handle EOF by limiting count to the filesize
    int amountAlreadyDelivered = (fcb->currentBlk * B_CHUNK_SIZE) - remainingBytesInMyBuffer;
    printf("Amount already delivered: %d\n", amountAlreadyDelivered);
    printf("File size: %d\n", fcb->fileSize);
    if ((count + amountAlreadyDelivered) > fcb->fileSize)
    {
        count = fcb->fileSize - amountAlreadyDelivered;
        if (count < 0)
        {
            printf("Error: negative count\n");
            return -1;
        }
    }

    // part 1 is currently in the buffer and available to satisfy the request
    if (remainingBytesInMyBuffer >= count)
    { // Entire request is satisfied by buffer amount
        printf("We can satisfy this request with just part 1.\n");
        part1 = count;
        part2 = 0; // Do not need to load anything else
        part3 = 0;
    }
    else
    { // Give the caller the rest of the buffer & calculate pt2 and 3
        printf("We can't satisfy this request with just part 1.\n");
        part1 = remainingBytesInMyBuffer;
        part3 = count - remainingBytesInMyBuffer;
        // If there are blocks we can copy directly to the user's buffer, calculate this
        numBlocksToCopy = part3 / B_CHUNK_SIZE;
        part2 = numBlocksToCopy * B_CHUNK_SIZE;
        // Set part3 to the remaining bytes left to copy
        // Part3 will be loaded into the intermediary buffer rather than the user's buf directly
        part3 -= part2;
    }

    printf("Part 1: %d\n", part1);
    printf("Part 2: %d\n", part2);
    printf("Part 3: %d\n", part3);

    if (part1 > 0)
    { // Copy part1 bytes to user buffer and increment internal buffer position
        memcpy(buffer, fcb->buf + fcb->index, part1);
        fcb->index += part1;
    }
    if (part2 > 0)
    { // Read from disk directly to user buffer
        blocksRead = discontinuousPartialRead(fcb->currentBlk, buffer + part1, numBlocksToCopy);
        // Get the location of the new current block
        for (int i = 0; i < numBlocksToCopy; i++)
        {
            fcb->currentBlk = fat[fcb->currentBlk];
        }
        // Update with the actual value of how much was read
        part2 = blocksRead * B_CHUNK_SIZE;
    }
    if (part3 > 0)
    { // Need to load the intermediary buffer
        blocksRead = discontinuousPartialRead(fcb->currentBlk, fcb->buf, 1);
        // Update with the actual value of how much was read
        bytesRead = blocksRead * B_CHUNK_SIZE;
        fcb->currentBlk = fat[fcb->currentBlk];
        // Reset buffer values
        fcb->index = 0;
        fcb->buflen = bytesRead;

        if (bytesRead < part3)
        { // Not enough left to satisfy read request from caller
            part3 = bytesRead;
        }
        if (part3 > 0)
        {
            memcpy(buffer + part1 + part2, fcb->buf + fcb->index, part3);
            fcb->index += part3;
        }
    }
    bytesReturned = part1 + part2 + part3;

    printf("\n[End b_read]. Returned %d bytes.\n", bytesReturned);

    return bytesReturned;
}

// Interface to Close the file
/**
 * int b_close(b_io_fd fd)
 *
 * TODO :: check initialized value, will be related to b_open & read
 */

int b_close(b_io_fd fd)
{
    // Check that fd is between 0 and (MAXFCBS-1)
    printf("\n[In b_close]\n");
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
    if (fcb->isDirty == 1)
    {
        int targetBlock = moveBlockIndex(fcb->fileLocation, fcb->filePointer / B_CHUNK_SIZE);
        int p1Write = discontinuousPartialWrite(targetBlock, fcb->buf, 1);
    }

    // Free the buffer
    if (fcb->buf != NULL)
    {
        printf("Freeing the buffer. \n");
        free(fcb->buf);
        fcb->buf = NULL;
    }
    if (fcb->fileSize > 0 && fcb->directoryEntry != NULL)
    {
        fcb->directoryEntry;
        fcb->directoryEntry->size = fcb->fileSize;
        fcb->directoryEntry->timeModified = time(NULL);

        printf("DE name: %s\n", fcb->directoryEntry->name);
        printf("DE location: %d\n", fcb->directoryEntry->location);
        printf("DE size: %d\n", fcb->directoryEntry->size);

        if (writeDir(fcb->directoryEntry) < 0)
        {
            printf("Error writing directory entry.\n");
            return -1;
        }
        freeDir(fcb->directoryEntry);
    }

    // Reset the file control block
    printf("Resetting FCB. \n");
    fcb->fileDescriptor = -1;
    fcb->filePointer = 0;
    fcb->fileSize = 0;
    fcb->blockSize = 0;

    printf("\n[End b_close]\n");

    return 0; // Success
}

int b_move(char *pathnameSrc, char *pathnameDest)
{
    DE *retParent1;
    int index1 = 0;
    char lastElemName1[MAX_NAME_LENGTH];
    printf("Parsing source path. \n");
    int result1 = parsePath(pathnameSrc, &retParent1, &index1, lastElemName1);
    if (result1 < 0)
    {
        printf("Invalid path. \n");
        return -1;
    }
    if (index1 < 0 || retParent1[index1].isDirectory != 0)
    {
        printf("File does not exist in the specified path.\n");
        return -1;
    }
    DE *retParent2;
    int index2 = 0;
    char lastElemName2[MAX_NAME_LENGTH];
    printf("Parsing destination path. \n");
    int result2 = parsePath(pathnameDest, &retParent2, &index2, lastElemName2);
    if (result2 < 0)
    {
        printf("Invalid path. \n");
        return -1;
    }
    printf("Successfully parsed path. \n");

    printf("Loading destination directory. \n");
    DE *destDirectory = loadDir(&retParent2[index2]);
    if (destDirectory == NULL)
    {
        printf("Error loading source directory.\n");
        freeDir(retParent1);
        freeDir(retParent2);
        return -1;
    }

    printf("Finding unused directory in the destination. \n");
    int index = firstUnusedDirEntry(destDirectory);
    if (index < 2)
    {
        printf("Error finding unused directory entry.\n");
        freeDir(destDirectory);
        freeDir(retParent1);
        freeDir(retParent2);
        return -1;
    }
    printf("Populating destination directory entry. \n");
    strcpy(destDirectory[index].name, retParent1[index1].name);
    printf("Directory name is now: %s\n", destDirectory[index].name);
    destDirectory[index].isDirectory = 0;
    destDirectory[index].location = retParent1[index1].location;
    destDirectory[index].size = retParent1[index1].size;
    destDirectory[index].timeCreated = retParent1[index1].timeCreated;
    destDirectory[index].timeModified = time(NULL);

    printf("Populated dest. directory entry. Writing dest. directory entry.\n");
    if (writeDir(destDirectory) < 0)
    {
        printf("Error writing destination directory.\n");
        return -1;
    }
    /*printf("Dumping destination directory after move:\n");
    for (int i = 0; i < 50; i++)
    {
        printf("[%d] name=%s, isDirectory=%d, location=%d, size=%d\n", i,
               destDirectory[i].name, destDirectory[i].isDirectory,
               destDirectory[i].location, destDirectory[i].size);
    }*/
    printf("Resetting old parent to be unused.\n");
    strncpy(retParent1[index1].name, "\0", MAX_NAME_LENGTH);
    memset(&retParent1[index1], 0, sizeof(DE));
    printf("Writing src. directory entry.\n");
    if (writeDir(retParent1) < 0)
    {
        printf("Error writing source directory.\n");
        return -1;
    }
    if (retParent2 != destDirectory && retParent2 != retParent1)
    {
        freeDir(retParent2);
    }
    freeDir(destDirectory);
    freeDir(retParent1);
    return 0;
}
