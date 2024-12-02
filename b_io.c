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
 * Description::
 * This file (b_io.c) implements the basic file I/O operations for our
 * filesystem project. It provides functions to open, read, write,
 * seek, and close files. The file control block (FCB) structure is
 * used to manage open files and their associated buffers. The key
 * functions in this file include:
 *
 * Functions:
 * - b_init()   - Initializes the file system by setting up the FCB array.
 * - b_getFCB() - Retrieves a free FCB element.
 * - b_open()   - Opens a file and retrive data from disk.
 * - b_read()   - Reads data from an open file into a buffer.
 * - b_write()  - Writes data from a buffer to an open file.
 * - b_seek()   - Changes the file pointer position for an open file.
 * - b_close()  - Closes an open file and frees associated resources.
 *
 * These functions work together to provide a basic interface for
 * file operations, allowing users to interact with the filesystem
 * by opening, reading, writing, seeking, and closing files. The
 * implementation ensures that data is buffered efficiently and
 * handles various file operations correctly.
 *
 * Flags:
 * - O_CREAT    - Creates a new file if it does not exist.
 *                (ignored if the file does exist)
 * - O_TRUNC    - Set the file length to 0 (truncates all data)
 * - O_APPEND   - Sets the file position to the end of the file
 *                (Same as doing a seek 0 from SEEK_END)
 * - O_RDONLY   - File can only do read/seek operations
 * - O_WRONLY   - File can only do write/seek operations
 * - O_RDWR     - File can be read or written to.
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
 * b_io_fd b_open(char *filename, int flags)
 *
 * Description: Opens a file, retrive data from disk to memory and returns a
 * file descriptor
 *
 * @param filename: The name of the file to open
 * @param flags: The flags to open the file with
 * @return: The file descriptor of the opened file
 */
b_io_fd b_open(char *filename, int flags)
{
    b_io_fd returnFd;

    DE *retParent;
    int index = 0;
    char lastElemName[MAX_NAME_LENGTH];
    int result = parsePath(filename, &retParent, &index, lastElemName);
    if (result < 0)
    {
        freeDir(retParent);
        return -1;
    }
    if (index > 0 && retParent[index].isDirectory)
    {
        freeDir(retParent);
        return -1;
    }
    b_fcb *fcb = (b_fcb *)malloc(sizeof(b_fcb));
    if (fcb == NULL)
    {
        printf("[[Critical]] Error occured while opening file. \n");
        freeDir(retParent);
        return -1;
    }
    memset(fcb, 0, sizeof(b_fcb));

    time_t now = time(NULL);
    // Create the file if it doesn't exist
    if (flags & O_CREAT)
    {
        if (index < 0)
        {
            index = firstUnusedDirEntry(retParent);
            if (index < 0) // No more unused DEs in the parent
            {
                freeDir(retParent);
                free(fcb);
                fcb = NULL;
                return -1;
            }
            strcpy(retParent[index].name, lastElemName);
            retParent[index].size = 0;
            retParent[index].location = 0;
            retParent[index].timeCreated = now;
        }
    }
    // Truncate existing file
    if (flags & O_TRUNC)
    {
        if (index < 0) // File not found
        {
            freeDir(retParent);
            free(fcb);
            fcb = NULL;
            return -1;
        }
        int numBytesToRelease = retParent[index].size;
        int blocksToRelease = (numBytesToRelease + (vcb->blockSize - 1)) / vcb->blockSize;

        retParent[index].isDirectory = 0;
        retParent[index].size = 0;
        retParent[index].location = 0;
    }

    // Handle O_RDONLY flag
    //  File can only do read/seek operations
    if (flags & O_RDONLY)
    {
        if (index < 0) // File not found
        {
            freeDir(retParent);
            free(fcb);
            fcb = NULL;
            return -1;
        }
    }
    // Handle O_WRONLY flag
    //  File can only do write/seek operations
    if (flags & O_WRONLY)
    {
        if (index < 0) // File not found
        {
            freeDir(retParent);
            free(fcb);
            fcb = NULL;
            return -1;
        }
    }

    // Handle O_RDWR flag
    // File can be read or written to
    if (flags & O_RDWR)
    {
        if (index < 0)
        {
            freeDir(retParent);
            free(fcb);
            fcb = NULL;
            return -1;
        }
    }
    returnFd = b_getFCB(); // get our own file descriptor
                           // check for error - all used FCB's
    if (returnFd < 0)
    {
        freeDir(retParent);
        free(fcb);
        fcb = NULL;
        return -1;
    }
    fcb->blockSize = vcb->blockSize;
    fcb->currentBlk = retParent[index].location;
    fcb->deIndex = index;
    fcb->directoryEntry = retParent;
    fcb->fileDescriptor = returnFd;
    fcb->fileLocation = retParent[index].location;
    fcb->filePointer = 0;
    fcb->fileSize = retParent[index].size;
    fcb->index = 0;
    fcb->isDirty = 0; // Haven't loaded buffer yet.
    char *buf = malloc(B_CHUNK_SIZE);
    if (buf == NULL)
    {
        printf("[[Critical]] Error occured while opening file. \n");
        freeDir(retParent);
        free(fcb);
        fcb = NULL;
        return -1;
    }
    memset(buf, 0, B_CHUNK_SIZE);

    fcb->buf = buf;
    fcb->buflen = 0;
    if (writeDir(retParent) < 1)
    {
        free(buf);
        buf = NULL;
        freeDir(retParent);
        free(fcb);
        fcb = NULL;
        return -1;
    }
    fcbArray[returnFd] = *fcb;
    if (flags & O_APPEND)
    {
        b_seek(returnFd, 0, SEEK_END);
    }

    return (returnFd); // all set
}

/**
 * int b_seek(b_io_fd fd, off_t offset, int whence)
 *
 * Description: To move the file pointer to a specified location in the file,
 * aka, seek.
 *
 * @param fd The file descriptor
 * @param offset The offset from the position specified by whence
 * @param whence The position from which to start
 * @return The new file pointer on success, -1 on error
 */
int b_seek(b_io_fd fd, off_t offset, int whence)
{
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
        printf("[[Critical]] Error occured while reading file\n");
        return -1; // Error reading file
    }
    fcb->index = blockOffset;

    return newPointer; // Return the new file pointer position
}

/**
 * int b_write(b_io_fd fd, char *buffer, int count)
 *
 * Description: Writes a specified number of bytes to a file.
 *
 * @param fd The file descriptor of the file to write to.
 * @param buffer The pointer to the buffer containing the data to write.
 * @param count The number of bytes to write.
 * @return The number of bytes written, or -1 on error.
 */
int b_write(b_io_fd fd, char *buffer, int count)
{
    // Validate the file descriptor
    if (fd < 0 || fd >= MAXFCBS || fcbArray[fd].buf == NULL)
    {
        return -1; // Invalid fd
    }

    b_fcb *fcb = &fcbArray[fd];

    // Check if the file is read-only
    if (fcb->accessFlags & O_RDONLY || fcb->accessFlags & O_RDWR)
    {
        printf("[[Critical]] File is read-only. Cannot write to it.\n");
        return -1;
    }

    // Check if the file is writable
    if (fcb->fileDescriptor < 0)
    {
        printf("[[Critical]] File is not writable.\n");
        return -1;
    }

    if (fcb->fileSize == 0)
    {
        fcb->fileLocation = allocateBlocks(1); // Allocate a block for the file;
        (fcb->directoryEntry + fcb->deIndex)->location = fcb->fileLocation;
        fcb->currentBlk = fcb->fileLocation;
    }

    // P0.5:
    // If count is smaller than the remaining bytes in the current block, write
    // and return.
    if (count < B_CHUNK_SIZE - (fcb->filePointer % B_CHUNK_SIZE))
    {
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
        if (extendChain((excessBytes + B_CHUNK_SIZE - 1) / B_CHUNK_SIZE, fcb->fileLocation) < 0)
        {
            return -1;
        }
    }

    // p1:
    // Fill up all the available bytes in the current block, and write it
    // to disk
    targetBlock = moveBlockIndex(fcb->fileLocation, fcb->filePointer / B_CHUNK_SIZE);

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
        return -1;
    }
    targetBlock = moveBlockIndex(targetBlock, 1);

    // p2:
    // Calculate how many blocks are needed to store the remaining, and write
    // the blocks to disk at once
    int p2NeededBlocks = count / B_CHUNK_SIZE;
    if (p2NeededBlocks != 0)
    {
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
            return -1;
        }
    }

    // p3:
    // Write the remaining bytes left, based on the results from p1 and p2.
    // Instead of writing to disk, we make the block dirty and save it for close
    int p3Read = discontinuousPartialRead(targetBlock, fcb->buf, 1);
    if (p3Read == -1)
    {
        return -1;
    }
    memcpy(fcb->buf, buffer + p1 + p2, count);
    fcb->filePointer += count;
    if (fcb->fileSize < fcb->filePointer)
    {
        fcb->fileSize = fcb->filePointer;
    }
    fcb->isDirty = 1;

    time_t now = time(NULL);
    fcb->directoryEntry->timeModified = now;

    return count + p1 + p2;
}

/**
 * int b_read(b_io_fd fd, char *buffer, int count)
 *
 * Description: Reads a specified number of bytes from a file in the disk.
 *
 * @param fd The file descriptor of the file to read from.
 * @param buffer The pointer to the buffer to store the read data.
 * @param count The number of bytes to read
 * @return The number of bytes read, or -1 on error.
 */
int b_read(b_io_fd fd, char *buffer, int count)
{
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
        printf("[[Critical]] File is write-only. Cannot read from it.\n");
        return -1;
    }

    // Check if the file is open
    if (fcb->fileDescriptor == -1)
    {
        return -1; // File not open
    }

    int blocksRead;
    int bytesRead;
    int bytesReturned;
    int part1, part2, part3;
    int numBlocksToCopy;
    int remainingBytesInMyBuffer;

    // Calculate bytes available in the buffer
    remainingBytesInMyBuffer = fcb->buflen - fcb->index;
    // Handle EOF by limiting count to the filesize
    int amountAlreadyDelivered = fcb->filePointer;
    if ((count + amountAlreadyDelivered) > fcb->fileSize)
    {
        count = fcb->fileSize - amountAlreadyDelivered;
        if (count < 0)
        {
            return -1;
        }
    }

    // part 1 is currently in the buffer and available to satisfy the request
    if (remainingBytesInMyBuffer >= count)
    { // Entire request is satisfied by buffer amount
        part1 = count;
        part2 = 0; // Do not need to load anything else
        part3 = 0;
    }
    else
    { // Give the caller the rest of the buffer & calculate pt2 and 3
        part1 = remainingBytesInMyBuffer;
        part3 = count - remainingBytesInMyBuffer;
        // If there are blocks we can copy directly to the user's buffer,
        // calculate this
        numBlocksToCopy = part3 / B_CHUNK_SIZE;
        part2 = numBlocksToCopy * B_CHUNK_SIZE;
        // Set part3 to the remaining bytes left to copy
        // Part3 will be loaded into the intermediary buffer rather than the
        // user's buf directly
        part3 -= part2;
    }

    if (part1 > 0)
    { // Copy part1 bytes to user buffer and increment internal buffer position
        memcpy(buffer, fcb->buf + fcb->index, part1);
        fcb->index += part1;
    }
    if (part2 > 0)
    { // Read from disk directly to user buffer
        blocksRead = discontinuousPartialRead(fcb->currentBlk, buffer + part1, numBlocksToCopy);
        // Get the location of the new current block
        fcb->currentBlk = moveBlockIndex(fcb->currentBlk, numBlocksToCopy);
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
    fcb->filePointer += bytesReturned;

    return bytesReturned;
}

// Interface to Close the file
/**
 * int b_close(b_io_fd fd)
 *
 * Description: Close file and clean up resources, and writes data to disk when
 * current working block is empty.
 *
 * @param fd The file descriptor to close
 * @return 0 on success, -1 on error
 */

int b_close(b_io_fd fd)
{
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
    if (fcb->isDirty == 1)
    {
        int targetBlock = moveBlockIndex(fcb->fileLocation, fcb->filePointer / B_CHUNK_SIZE);
        int p1Write = discontinuousPartialWrite(targetBlock, fcb->buf, 1);
    }

    // Free the buffer
    if (fcb->buf != NULL)
    {
        free(fcb->buf);
        fcb->buf = NULL;
    }
    if (fcb->fileSize > 0 && fcb->directoryEntry != NULL)
    {
        DE *paDE = fcb->directoryEntry;
        (paDE + fcb->deIndex)->size = fcb->fileSize;
        (paDE + fcb->deIndex)->timeModified = time(NULL);
        if (writeDir(fcb->directoryEntry) < 0)
        {
            return -1;
        }
        freeDir(fcb->directoryEntry);
    }

    // Reset the file control block
    fcb->fileDescriptor = -1;
    fcb->filePointer = 0;
    fcb->fileSize = 0;
    fcb->blockSize = 0;
    return 0; // Success
}

/**
 * int b_move(char *pathnameSrc, char *pathnameDest)
 *
 * Description: Moves a file from one directory to another.
 *
 * @param pathnameSrc The path of the source file.
 * @param pathnameDest The path of the destination directory.
 * @return 0 on success, -1 on error.
 */
int b_move(char *pathnameSrc, char *pathnameDest)
{
    DE *retParent1;
    int index1 = 0;
    char lastElemName1[MAX_NAME_LENGTH];
    int result1 = parsePath(pathnameSrc, &retParent1, &index1, lastElemName1);
    if (result1 < 0)
    {
        printf("[[Critical]] Insufficient parameters. \n");
        return -1;
    }
    if (index1 < 0)
    {
        printf("[[Critical]] Target file does not exist.\n");
        return -1;
    }
    if (retParent1[index1].isDirectory != 0)
    {
        printf("[[Critical]] Cannot move a directory.\n");
        return -1;
    }
    DE *retParent2;
    int index2 = 0;
    char lastElemName2[MAX_NAME_LENGTH];
    int result2 = parsePath(pathnameDest, &retParent2, &index2, lastElemName2);
    if (result2 < 0)
    {
        printf("[[Critical]] Destination is empty. \n");
        return -1;
    }
    DE *destDirectory;
    if (index2 < 0)
    {
        destDirectory = retParent2;
    }
    else
    {
        destDirectory = loadDir(&retParent2[index2]);
    }

    if (destDirectory == NULL)
    {
        printf("[[Critical]] Invalid destination path. \n");
        freeDir(retParent1);
        freeDir(retParent2);
        return -1;
    }

    int index = firstUnusedDirEntry(destDirectory);
    if (index < 2)
    {
        freeDir(destDirectory);
        freeDir(retParent1);
        freeDir(retParent2);
        return -1;
    }
    if (index2 < 0)
    {
        strcpy(destDirectory[index].name, lastElemName2);
    }
    else
    {
        strcpy(destDirectory[index].name, retParent1[index1].name);
    }

    destDirectory[index].isDirectory = 0;
    destDirectory[index].location = retParent1[index1].location;
    destDirectory[index].size = retParent1[index1].size;
    destDirectory[index].timeCreated = retParent1[index1].timeCreated;
    destDirectory[index].timeModified = time(NULL);

    if (writeDir(destDirectory) < 0)
    {
        return -1;
    }

    retParent1 = loadDir(retParent1);
    strncpy(retParent1[index1].name, "\0", MAX_NAME_LENGTH);
    memset(&retParent1[index1], 0, sizeof(DE));

    if (writeDir(retParent1) < 0)
    {
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
