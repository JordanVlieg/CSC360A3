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

	FILE* hostFile;
	hostFile = fopen(hostFileName,"r");

	if(diskFile == NULL)
	{
		printf("File not found error\n");
		return 0;
	}

	if(hostFile == NULL)
	{
		printf("File not found error\n");
		return 0;
	}
	fclose(hostFile);

	int blockSize = readBlockSize(diskFile);
	//int blockCount = readBlockCount(diskFile);
	int fatStart = readFATStart(diskFile);
	int numFatBlocks = readFATBlocks(diskFile);
	int rootDirStart = readDirStart(diskFile);
	int numRootDirBlocks = readDirBlocks(diskFile);

	int* fatInfo;
	fatInfo = findFATBlocks(diskFile, fatStart, numFatBlocks, blockSize);

	int availableBlocks = fatInfo[0];

	fclose(diskFile);

	putFileOnClient(diskFileName, hostFileName, availableBlocks, rootDirStart, numRootDirBlocks, blockSize, fatStart, numFatBlocks);
	return 1;
}
