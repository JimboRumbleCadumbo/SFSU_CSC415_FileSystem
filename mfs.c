/**************************************************************
 * Class::  CSC-415-03 Fall 2024
 * Name:: Yu-Ming Chen, Ria Thakker, Yuquan Xu, Rishita Meharishi
 * Student IDs:: 923313947, 922593253, 920390312, 922249026
 * GitHub-Name:: JimboRumbleCadumbo
 * Group-Name:: Bytes Busters
 * Project:: Basic File System
 *
 * File:: mfs.c
 *
 * Description::
 *	This is the file system interface.
 *	This is the interface needed by the driver to interact with
 *	your filesystem.
 *
 **************************************************************/

#include "mfs.h"

// Key directory functions
int fs_mkdir(const char *pathname, mode_t mode)
{
    if (pathname == NULL || strlen(pathname) == 0)
    {
        return -1; // Empty path
    }
    DE *retParent;
    int index = 0;
    char lastElemName[MAX_NAME_LENGTH];
    char *path = strdup(pathname);
    int result = parsePath(path, &retParent, &index, lastElemName);
    free(path);
    path = NULL;
    if (result < 0 || index > 0)
    {
        return -1; // Invalid path / parse error / directory already exists
    }
    printf("Finished parsing path. %p is the root; %p is retParent.\n", root, retParent);
    DE *loadedDir = loadDir(retParent);
    if (loadedDir == NULL)
    {
        freeDir(retParent);
        printf("Error loading directory.\n");
        return -1; // error loading
    }
    int idx = firstUnusedDirEntry(loadedDir);
    printf("The first unused directory entry in the parent directory is %d.\n", idx);
    DE *newDir = createDirectory(ENTRIES_IN_DIR, loadedDir);
    if (newDir == NULL)
    {
        printf("Creating directory failed.\n");
        freeDir(retParent);
        return -1;
    }
    strcpy(loadedDir[idx].name, lastElemName);
    loadedDir[idx].isDirectory = 1;
    loadedDir[idx].location = newDir->location;
    loadedDir[idx].size = newDir->size;
    loadedDir[idx].timeCreated = newDir->timeCreated;
    loadedDir[idx].timeModified = newDir->timeModified;
    if (writeDir(loadedDir) < 1)
    {
        printf("Failed to write directories.\n");
        freeDir(retParent);
        return -1;
    }
    freeDir(retParent);
    printf("Success.\n");
    return 0;
}

int fs_rmdir(const char *pathname)
{
    printf("\nfs_rmdir \n");
    DE *retParent;
    int index = 0;
    char lastElemName[MAX_NAME_LENGTH];
    char *path = strdup(pathname);
    int result = parsePath(path, &retParent, &index, lastElemName);
    free(path);
    path = NULL;
    if (result < 0 || index < 0)
    {
        // Directory not found
        printf("Cannot remove directory, invalid path or directory not found.\n");
        return -1; // Invalid path / parse error / directory already exists
    }
    printf("Starting remove directory on %s\n", retParent[index].name);
    DE *target = loadDir(&retParent[index]);
    if (isDirEmpty(target) == 0)
    {
        printf("Cannot remove directory, directory is not empty.\n");
        freeDir(retParent);
        return -1;
    }

    int releaseResult = releaseBlocks(target->size / vcb->blockSize, target->location);
    if (releaseResult < 0)
    {
        printf("Failed to release blocks.\n");
        freeDir(retParent);
        return -1;
    }

    freeDir(target);

    strncpy(retParent[index].name, "\0", MAX_NAME_LENGTH);
    memset(&retParent[index], 0, sizeof(DE));

    int writeBackResult = writeDir(retParent);
    if (writeBackResult < 0)
    {
        printf("Failed to write back directory.\n");
        freeDir(retParent);
        return -1;
    }

    freeDir(retParent);
    return 0;
}

// Directory iteration functions
fdDir *fs_opendir(const char *pathname)
{
    printf("Starting open directory on %s\n", pathname);
    DE *retParent;
    int index = 0;
    char lastElemName[MAX_NAME_LENGTH];
    char *path = strdup(pathname);
    if (path == NULL)
    {
        return NULL;
    }
    int result = parsePath(path, &retParent, &index, lastElemName);
    free(path);
    path = NULL;
    if (result < 0 || index < 0)
    {
        printf("Error parsing path.\n");
        return NULL;
    }
    printf("Successfully parsed path.\n");
    fdDir *openedDir = (fdDir *)malloc(sizeof(fdDir));
    if (openedDir == NULL)
    {
        printf("Error allocating memory for fdDir struct.\n");
        return NULL;
    }
    printf("Successfully allocated memory for fdDir struct.\n");
    openedDir->d_reclen = sizeof(fdDir);
    DE *loadedDir = loadDir(&retParent[index]);
    if (loadedDir == NULL)
    {
        printf("Error loading directory.\n");
        free(openedDir);
        openedDir = NULL;
        return NULL;
    }
    printf("Successfully loaded directory.\n");
    openedDir->directory = loadedDir;
    struct fs_diriteminfo *di = (struct fs_diriteminfo *)malloc(sizeof(struct fs_diriteminfo));
    if (di == NULL)
    {
        printf("Error allocating fs_diriteminfo struct.\n");
        free(openedDir);
        openedDir = NULL;
        freeDir(loadedDir);
        loadedDir = NULL;
    }
    strcpy(di->d_name, pathname);
    openedDir->dirEntryPosition = 0;
    openedDir->di = di;
    printf("Successfully opened directory.\n");
    return openedDir;
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp)
{

    int pos = dirp->dirEntryPosition;
    // printf("fs_readdir - pos: %d, dirEntryPosition: %d, entry name: %s\n",
    // pos, dirp->dirEntryPosition, dirp->directory[pos].name);
    if (pos < ENTRIES_IN_DIR && dirp->directory[pos].name[0] != '\0')
    {
        strncpy(dirp->di->d_name, dirp->directory[pos].name, strlen(dirp->directory[pos].name));
        dirp->di->d_name[sizeof(dirp->di->d_name) - 1] = '\0';
        dirp->di->d_reclen = sizeof(struct fs_diriteminfo);
        dirp->di->timeCreated = dirp->directory[pos].timeCreated;
        dirp->di->timeModified = dirp->directory[pos].timeModified;
        dirp->dirEntryPosition++;
        return dirp->di;
    }
    return NULL;
}

int fs_closedir(fdDir *dirp)
{
    free(dirp->di);
    dirp->di = NULL;
    freeDir(dirp->directory);
    dirp->directory = NULL;

    dirp->d_reclen = 0;
    dirp->dirEntryPosition = 0;

    free(dirp);
    dirp = NULL;
    printf("Successfully closed directory.\n");
    return 0; // Close success
}

// Misc directory functions
char *fs_getcwd(char *pathname, size_t size)
{
    // Copy the current working directory string to the provided buffer
    strncpy(pathname, cwdString, size - 1);
    pathname[size - 1] = '\0'; // Ensure null-termination

    return pathname; // Return the buffer containing the current working directory
}

int fs_setcwd(char *pathname)
{ // linux chdir
    printf("Starting set CWD function.\n");
    if (pathname == NULL || strlen(pathname) == 0 || strcmp(pathname, "/") == 0)
    {
        printf("Setting path to root directory.\n");
        freeDir(cwd);
        cwd = root;
        char *rootPath = "/";
        strncpy(cwdString, rootPath, MAX_PATH_LENGTH);
        cwdString[strlen(cwdString)] = '\0';
        return 0; // Path is just root directory
    }

    // Validate the input path & confirm last element exists
    DE *retParent;
    int index = 0;
    char lastElemName[MAX_NAME_LENGTH];
    int result = parsePath(pathname, &retParent, &index, lastElemName);
    if (index == -1 || retParent == NULL)
    {
        printf("Index is -1 or retParent is null.\n");
        return -1; // Safety check
    }
    if (result == -1 || retParent[index].isDirectory == 0)
    {
        printf("Invalid path or directory not found or not directory.\n");
        return -1; // Invalid path or not directory or directory not found
    }

    printf("Loading directory.\n");
    DE *temp = loadDir(&retParent[index]);
    if (temp < 0)
    {
        printf("Error loading directory.\n");
        return -1;
    }
    // Free the previous cwd
    freeDir(cwd);
    cwd = temp;

    // Start to tokenize the string to update the cwdString
    char newCwdString[MAX_PATH_LENGTH];
    char tempCwdString[MAX_PATH_LENGTH]; // To preserve the original path for tokenization
    char *tokens[MAX_PATH_LENGTH];
    int tokenCount = 0;

    // If the pathname is absolute, start from root
    if (pathname[0] == '/')
    {
        strncpy(newCwdString, pathname, MAX_PATH_LENGTH - 1);
    }
    else
    {
        strncpy(newCwdString, cwdString, MAX_PATH_LENGTH - 1);
        // Only add "/" if cwdString does not already end with "/"
        if (newCwdString[strlen(newCwdString) - 1] != '/')
        {
            strncat(newCwdString, "/", MAX_PATH_LENGTH - strlen(newCwdString) - 1);
        }
        strncat(newCwdString, pathname, MAX_PATH_LENGTH - strlen(newCwdString) - 1);
    }

    // Copy the original path into tempCwdString for tokenization
    strncpy(tempCwdString, newCwdString, MAX_PATH_LENGTH - 1);
    tempCwdString[MAX_PATH_LENGTH - 1] = '\0'; // Ensure null termination
    printf("Current path: %s\n", tempCwdString);

    // Tokenize and process the path
    char *token;
    char *saveptr;
    token = strtok_r(tempCwdString, "/", &saveptr);
    while (token != NULL)
    {
        if (strcmp(token, ".") == 0)
        {
            // Ignore "."
        }
        else if (strcmp(token, "..") == 0)
        {
            // Go up one directory
            if (tokenCount > 0)
            {
                tokenCount--;
            }
        }
        else
        {
            // Add token to the list
            tokens[tokenCount] = token;
            tokenCount++;
        }
        token = strtok_r(NULL, "/", &saveptr);
    }

    // Rebuild newCwdString based on tokens
    newCwdString[0] = '/'; // Root
    newCwdString[1] = '\0';
    for (int i = 0; i < tokenCount; i++)
    {
        strncat(newCwdString, tokens[i], MAX_PATH_LENGTH - strlen(newCwdString) - 1);
        if (i < tokenCount - 1)
        {
            strncat(newCwdString, "/", MAX_PATH_LENGTH - strlen(newCwdString) - 1);
        }
    }
    // Update cwdString
    strncpy(cwdString, newCwdString, MAX_PATH_LENGTH - 1);
    cwdString[MAX_PATH_LENGTH - 1] = '\0';

    printf("New CWD string: %s\n", cwdString);
    return 0;
}

int fs_isFile(char *filename)
{
    DE *retParent;
    int index = 0;
    char lastElemName[MAX_NAME_LENGTH];
    int result = parsePath(filename, &retParent, &index, lastElemName);
    if (result < 0 || index < 0 || lastElemName == NULL || retParent == NULL)
    {
        freeDir(retParent);
        return 0; // Failure
    }
    int isDir = retParent[index].isDirectory;
    freeDir(retParent);
    return (!isDir);
}

int fs_isDir(char *pathname)
{
    DE *retParent;
    int index = 0;
    char lastElemName[MAX_NAME_LENGTH];
    char *path = strdup(pathname);
    int result = parsePath(path, &retParent, &index, lastElemName);
    free(path);
    if (result < 0 || index < 0 || lastElemName == NULL || retParent == NULL)
    {
        freeDir(retParent);
        return 0; // Not directory
    }
    int isDir = retParent[index].isDirectory;
    freeDir(retParent);
    return isDir;
}

// Function to retrieve file or directory data
int fs_stat(const char *path, struct fs_stat *buf)
{
    if (path == NULL || strlen(path) == 0)
    {
        return -1;
    }
    DE *retParent;
    int index = 0;
    char lastElemName[MAX_NAME_LENGTH];
    // Copy path to pass into parsePath so we can strtok it
    char *pathname = strdup(path);
    int result = parsePath(pathname, &retParent, &index, lastElemName);
    free(pathname);
    pathname = NULL;
    if (result < 0)
    {
        freeDir(retParent);
        return -1; // Invalid path
    }
    if (retParent == NULL || lastElemName == NULL || index < 0)
    {
        freeDir(retParent);
        return -1; // No parent / no last element name / dir does not exist in parent
    }
    buf->st_size = retParent[index].size; // Assumed the size matches d_reclen
    buf->st_blksize = vcb->blockSize;
    buf->st_blocks = (buf->st_size + (vcb->blockSize - 1)) / (vcb->blockSize);

    buf->st_createtime = retParent[index].timeCreated;
    buf->st_modtime = retParent[index].timeModified;

    // We aren't tracking access time in our DE struct (structs.h)
    // buf->st_accesstime = time(NULL); // Placeholder

    // 1 for Directory, 0 for File
    buf->st_mode = retParent[index].isDirectory;
    freeDir(retParent);
    return 0;
}

/**
 * Waiting for b_open & b_close to finish for testing, might need debug.
 */
int fs_delete(char *filename)
{ // removes a file
    printf("\npfs_delete \n");
    DE *retParent;
    int index = 0;
    char lastElemName[MAX_NAME_LENGTH];
    char *path = strdup(filename); // need to change this into the file name location
    int result = parsePath(path, &retParent, &index, lastElemName);

    free(path);
    path = NULL;
    if (result < 0 || index < 0)
    {
        // Directory not found
        printf("Cannot remove file. Invalid path or file not found.\n");
        freeDir(retParent);
        return -1; // Invalid path / parse error / directory already exists
    }
    printf("Starting remove file on %s\n", retParent[index].name);
    if (retParent[index].size > 0) {
    DE *target = loadDir(&retParent[index]);

    int releaseResult = releaseBlocks(target->size / vcb->blockSize, target->location);
    if (releaseResult < 0)
    {
        printf("Failed to release blocks.\n");
        freeDir(retParent);
        return -1;
    }

    freeDir(target);
    }

    strncpy(retParent[index].name, "\0", MAX_NAME_LENGTH);
    int writeBackResult = writeDir(retParent);
    if (writeBackResult < 0)
    {
        printf("Failed to write back file.\n");
        freeDir(retParent);
        return -1;
    }
    freeDir(retParent);
    return 0;
}