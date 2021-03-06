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
		printf("File not found.\n");
		return 0;
	}

	int blockSize = readBlockSize(diskFile);
	int rootDirStart = readDirStart(diskFile);
	int numRootDirBlocks = readDirBlocks(diskFile);

	getFileInfo(diskFile, rootDirStart, numRootDirBlocks, blockSize);
	fclose(diskFile);
	return 0;
}
