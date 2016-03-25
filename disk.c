#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Constants.h"
#include "disk.h"

int charArrayToInt(unsigned char theBuffer[])
{
	int returnVal = (theBuffer[0] << 24) + (theBuffer[1] << 16) + (theBuffer[2] << 8) + theBuffer[3];
	return returnVal;
}

char* readFSID(FILE *diskFile)
{
	static char buffer[BLOCKSIZE_OFFSET - IDENT_OFFSET + 1];
	buffer[BLOCKSIZE_OFFSET - IDENT_OFFSET] = '\0';
	fseek ( diskFile , IDENT_OFFSET , SEEK_SET );
	//fgets(buffer, BLOCKSIZE_OFFSET - IDENT_OFFSET + 1, diskFile);
	fread(buffer, sizeof(char), BLOCKSIZE_OFFSET - IDENT_OFFSET, diskFile);
	return buffer;
}

int readBlockSize(FILE *diskFile)
{
	unsigned char buffer[BLOCKCOUNT_OFFSET - BLOCKSIZE_OFFSET + 1];
	fseek ( diskFile , BLOCKSIZE_OFFSET , SEEK_SET );
	//fgets(buffer, BLOCKCOUNT_OFFSET - BLOCKSIZE_OFFSET + 1, diskFile);
	fread(buffer, sizeof(unsigned char), BLOCKCOUNT_OFFSET - BLOCKSIZE_OFFSET, diskFile);
	int blockSize = (buffer[0] << 8) + buffer[1];
	return blockSize;
}

int readBlockCount(FILE *diskFile)
{
	unsigned char buffer[FATSTART_OFFSET - BLOCKCOUNT_OFFSET + 1];
	fseek ( diskFile , BLOCKCOUNT_OFFSET , SEEK_SET );
	//fgets(buffer, FATSTART_OFFSET - BLOCKCOUNT_OFFSET + 1, diskFile);
	fread(&buffer, sizeof(unsigned char), FATSTART_OFFSET - BLOCKCOUNT_OFFSET, diskFile);
	int blockCount = charArrayToInt(buffer);
	return blockCount;
}

int readFATStart(FILE *diskFile)
{
	unsigned char buffer[FATBLOCKS_OFFSET - FATSTART_OFFSET + 1];
	fseek ( diskFile , FATSTART_OFFSET , SEEK_SET );
	//fgets(buffer, FATBLOCKS_OFFSET - FATSTART_OFFSET + 1, diskFile);
	fread(&buffer, sizeof(unsigned char), FATBLOCKS_OFFSET - FATSTART_OFFSET, diskFile);
	int fatStart = charArrayToInt(buffer);
	return fatStart;
}

int readFATBlocks(FILE *diskFile)
{
	unsigned char buffer[ROOTDIRSTART_OFFSET - FATBLOCKS_OFFSET + 1];
	fseek ( diskFile , FATBLOCKS_OFFSET , SEEK_SET );
	//fgets(buffer, ROOTDIRSTART_OFFSET - FATBLOCKS_OFFSET + 1, diskFile);
	fread(buffer, sizeof(unsigned char), ROOTDIRSTART_OFFSET - FATBLOCKS_OFFSET, diskFile);
	int fatBlocks = charArrayToInt(buffer);
	return fatBlocks;
}

int readDirStart(FILE *diskFile)
{
	unsigned char buffer[ROOTDIRBLOCKS_OFFSET - ROOTDIRSTART_OFFSET + 1];
	fseek ( diskFile , ROOTDIRSTART_OFFSET , SEEK_SET );
	//fgets(buffer, ROOTDIRBLOCKS_OFFSET - ROOTDIRSTART_OFFSET + 1, diskFile);
	fread(buffer, sizeof(unsigned char), ROOTDIRBLOCKS_OFFSET - ROOTDIRSTART_OFFSET, diskFile);
	int dirStart = charArrayToInt(buffer);
	return dirStart;
}

int readDirBlocks(FILE *diskFile)
{
	unsigned char buffer[4 + 1];
	fseek ( diskFile , ROOTDIRBLOCKS_OFFSET , SEEK_SET );
	//fgets(buffer, ROOTDIRBLOCKS_OFFSET + 4 + 1, diskFile);
	fread(buffer, sizeof(unsigned char), 4, diskFile);
	int dirBlocks = charArrayToInt(buffer);
	return dirBlocks;
}

void findFATBlocks(FILE *diskFile, int fatStart, int numFATBlocks, int blockSize)
{
	unsigned char buffer[FAT_ENTRY_SIZE + 1];
	int available = 0;
	int reserved = 0;
	int allocated = 0;

	fseek ( diskFile , (fatStart * blockSize) , SEEK_SET );
	int fatPos;
	for(fatPos = 0; fatPos < numFATBlocks * blockSize; fatPos += FAT_ENTRY_SIZE)
	{
		//fgets(buffer, FAT_ENTRY_SIZE + 1, diskFile);
		fread(buffer, sizeof(unsigned char), FAT_ENTRY_SIZE, diskFile);
		int fatEntry = charArrayToInt(buffer);
		if(fatEntry == 0)
		{
			available = available + 1;
		}
		else if(fatEntry == 1)
		{
			reserved = reserved + 1;
		}
		else
		{
			allocated = allocated + 1;
		}
	}
	printf("\nFAT Information: \nFree Blocks: %d\nReserved Blocks: %d\nAllocated Blocks: %d\n", available, reserved, allocated);

	return;
}

char getStatusChar(char statusByte)
{
	char dataType = 0;
	if((statusByte & 1) == 0)
	{
		// Directory entry is free
		dataType = 0;
	}
	else
	{
		if((statusByte & 2) == 2)
		{
			dataType = 'F';
		}
		else if((statusByte & 4) == 4)
		{
			dataType = 'D';
		}
	}
	return dataType;
}

void getFileInfo(FILE *diskFile, int rootDirStart, int numRootDirBlocks, int blockSize)
{
	unsigned char buffer[DIRECTORY_ENTRY_SIZE + 1];

	//unsigned char* startingBlockPtr = &buffer[1];
	//unsigned char* numBlocksPtr = &buffer[5];
	unsigned char* fileSizePtr = &buffer[9];
	//unsigned char* createTimePtr = &buffer[13];
	//unsigned char* modifyTimePtr = &buffer[20];
	unsigned char* fileNamePtr = &buffer[27];
	//unsigned char* unusedPtr = &buffer[58];


	fseek ( diskFile , (rootDirStart * blockSize) , SEEK_SET );
	int dirPos;
	for(dirPos = 0; dirPos < numRootDirBlocks * blockSize; dirPos += DIRECTORY_ENTRY_SIZE)
	{
		//fgets(buffer, DIRECTORY_ENTRY_SIZE + 1, diskFile);
		fread(buffer, sizeof(unsigned char), DIRECTORY_ENTRY_SIZE, diskFile);

		char status = getStatusChar(buffer[0]);
		

		if(status != 0)
		{
			int fileSize = charArrayToInt(fileSizePtr); // Passes in a address that points to the fileSize portion of the directory entry

			int modifedYear = (buffer[20] << 8) + buffer[21];
			
			printf("%c ", status);
			printf("%10u ", fileSize);
			printf("%30s ", fileNamePtr);
			printf("%04d/%02d/%02d %02d:%02d:%02d", modifedYear, buffer[22], buffer[23], buffer[24], buffer[25], buffer[26]);
			printf("\n");
		}
	}
	return;
}

void getFileFromClient(FILE *diskFile, FILE *hostFile, char hostFileName[], int rootDirStart, int numRootDirBlocks, int blockSize)
{
	unsigned char directoryBuffer[DIRECTORY_ENTRY_SIZE + 1];

	unsigned char* startingBlockPtr = &directoryBuffer[1];
	//unsigned char* numBlocksPtr = &directoryBuffer[5];
	unsigned char* fileSizePtr = &directoryBuffer[9];
	//unsigned char* createTimePtr = &directoryBuffer[13];
	//unsigned char* modifyTimePtr = &directoryBuffer[20];
	char* fileNamePtr = &directoryBuffer[27];
	//unsigned char* unusedPtr = &directoryBuffer[58];

	int fileSize = -1;
	int startingFileBlock = -1;
	//int numFileBlocks = charArrayToInt(numBlocksPtr);

	//fseek ( diskFile , (startingFileBlock * blockSize) , SEEK_SET );

	fseek(diskFile, (rootDirStart * blockSize), SEEK_SET);

	fread(directoryBuffer, sizeof(unsigned char), DIRECTORY_ENTRY_SIZE, diskFile);
	
	int fileFound = 0;
	int dirPos;
	for(dirPos = 0; dirPos < numRootDirBlocks * blockSize; dirPos += DIRECTORY_ENTRY_SIZE)
	{
		char status = getStatusChar(directoryBuffer[0]);

		if(status != 0)
		{
			//printf("File name: %s", fileNamePtr);
			if(strcmp(hostFileName, fileNamePtr) == 0)
			{
				fileSize = charArrayToInt(fileSizePtr);
				//printf("File size: %u", fileSize);
				startingFileBlock = charArrayToInt(startingBlockPtr);
				fseek(diskFile, (startingFileBlock * blockSize), SEEK_SET);
				unsigned char fileBuffer[fileSize + 1];
				fread(fileBuffer, sizeof(unsigned char), fileSize, diskFile);
				fileBuffer[fileSize] = '\0';
				//printf("%s", fileBuffer);
				fwrite(fileBuffer, sizeof(unsigned char), fileSize, hostFile);
				fileFound = 1;
				break;
			}
		}
		fread(directoryBuffer, sizeof(unsigned char), DIRECTORY_ENTRY_SIZE, diskFile);
	}
	if(!fileFound)
	{
		printf("File not found.\n");
	}
}