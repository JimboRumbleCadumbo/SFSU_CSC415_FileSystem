
#include <time.h>
#include "fsFreeSpaceAllo.h"
#include "structs.h"    
#include "fsLow.h"

DE * createDirectory(int numEntries, DE *parent);
int writeDir(DE *dir);