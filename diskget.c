#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "Constants.h"
#include "disk.h"


int main ( int argc, char *argv[] )
{

	FILE *diskFile;
	FILE *hostFile;
	char *diskFileName = argv[1];
	char *hostFileName = argv[2];

	diskFile = fopen(diskFileName,"r");
	hostFile = fopen(hostFileName, "w");

	int blockSize = readBlockSize(diskFile);
	int blockCount = readBlockCount(diskFile);
	int fatStart = readFATStart(diskFile);
	int numFatBlocks = readFATBlocks(diskFile);
	int rootDirStart = readDirStart(diskFile);
	int numRootDirBlocks = readDirBlocks(diskFile);

	getFileFromClient(diskFile, hostFile, hostFileName, rootDirStart, numRootDirBlocks, blockSize);
	fclose(diskFile);
	fclose(hostFile);
	return 0;
}
