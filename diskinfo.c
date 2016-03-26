#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "Constants.h"
#include "disk.h"

int main ( int argc, char *argv[] )
{
	FILE *diskFile;
	char *filename = argv[1];
	diskFile = fopen(filename,"r");
	if(diskFile == NULL)
	{
		printf("File not found error\n");
		return 0;
	}
	int blockSize = readBlockSize(diskFile);
	int blockCount = readBlockCount(diskFile);
	int fatStart = readFATStart(diskFile);
	int numFatBlocks = readFATBlocks(diskFile);
	int rootDirStart = readDirStart(diskFile);
	int numRootDirBlocks = readDirBlocks(diskFile);

	printf("Super block information:  \n");
	printf("Block size: %d\n", blockSize);
	printf("Block count: %d\n", blockCount);
	printf("FAT starts: %d\n", fatStart);
	printf("FAT blocks: %d\n", numFatBlocks);
	printf("Root directory start: %d\n", rootDirStart);
	printf("Root directory blocks: %d\n", numRootDirBlocks);

	fflush(stdout);

	int* fatInfo;
	fatInfo = findFATBlocks(diskFile, fatStart, numFatBlocks, blockSize);

	int available = fatInfo[0];
	int reserved = fatInfo[1];
	int allocated = fatInfo[2];

	printf("\nFAT Information: \nFree Blocks: %d\nReserved Blocks: %d\nAllocated Blocks: %d\n", available, reserved, allocated);

	fclose(diskFile);
	return 0;
}