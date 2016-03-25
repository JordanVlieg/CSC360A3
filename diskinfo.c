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
	//char theBuffer[10];4294952960
	//fgets( theBuffer, 10, diskFile);
	//readFSID();
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

	findFATBlocks(diskFile, fatStart, numFatBlocks, blockSize);
	//getFileInfo(diskFile, rootDirStart, numRootDirBlocks, blockSize);
	return 0;
}