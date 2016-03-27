#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "Constants.h"
#include "disk.h"


int main ( int argc, char *argv[] )
{	
	char *diskFileName = argv[1];
	char *hostFileName = argv[2];

	FILE* diskFile;
	diskFile = fopen(diskFileName,"r");
	if(diskFile == NULL)
	{
		printf("File not found.\n");
		return 0;
	}

	int blockSize = readBlockSize(diskFile);
	int rootDirStart = readDirStart(diskFile);
	int numRootDirBlocks = readDirBlocks(diskFile);

	getFileFromClient(diskFileName, hostFileName, rootDirStart, numRootDirBlocks, blockSize);
	fclose(diskFile);
	return 1;
}
