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

	int blockSize = readBlockSize(diskFile);
	int blockCount = readBlockCount(diskFile);
	int fatStart = readFATStart(diskFile);
	int numFatBlocks = readFATBlocks(diskFile);
	int rootDirStart = readDirStart(diskFile);
	int numRootDirBlocks = readDirBlocks(diskFile);

	getFileInfo(diskFile, rootDirStart, numRootDirBlocks, blockSize);
	fclose(diskFile);
	return 0;
}
