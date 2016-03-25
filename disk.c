#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "Constants.h"
#include "disk.h"

FILE *diskFile;
char *filename = NULL;

int charArrayToInt(unsigned char* theBuffer)
{
	return (theBuffer[0] << 24) + (theBuffer[1] << 16) + (theBuffer[2] << 8) + theBuffer[3];
}

char* readFSID(FILE *diskFile)
{
	char* buffer = malloc(BLOCKSIZE_OFFSET - IDENT_OFFSET + 1);
	buffer[BLOCKSIZE_OFFSET - IDENT_OFFSET] = '\0';
	fseek ( diskFile , IDENT_OFFSET , SEEK_SET );
	//fgets(buffer, BLOCKSIZE_OFFSET - IDENT_OFFSET + 1, diskFile);
	fread(buffer, sizeof(char), BLOCKSIZE_OFFSET - IDENT_OFFSET, diskFile);
	return buffer;
}

int readBlockSize(FILE *diskFile)
{
	unsigned char* buffer = malloc(BLOCKCOUNT_OFFSET - BLOCKSIZE_OFFSET + 1);
	fseek ( diskFile , BLOCKSIZE_OFFSET , SEEK_SET );
	//fgets(buffer, BLOCKCOUNT_OFFSET - BLOCKSIZE_OFFSET + 1, diskFile);
	fread(buffer, sizeof(unsigned char), BLOCKCOUNT_OFFSET - BLOCKSIZE_OFFSET, diskFile);
	int blockSize = (buffer[0] << 8) + buffer[1];
	return blockSize;
}

int readBlockCount(FILE *diskFile)
{
	unsigned char* buffer = malloc(FATSTART_OFFSET - BLOCKCOUNT_OFFSET + 1);
	fseek ( diskFile , BLOCKCOUNT_OFFSET , SEEK_SET );
	//fgets(buffer, FATSTART_OFFSET - BLOCKCOUNT_OFFSET + 1, diskFile);
	fread(buffer, sizeof(unsigned char), FATSTART_OFFSET - BLOCKCOUNT_OFFSET, diskFile);
	int blockCount = charArrayToInt(buffer);
	return blockCount;
}

int readFATStart(FILE *diskFile)
{
	unsigned char* buffer = malloc(FATBLOCKS_OFFSET - FATSTART_OFFSET + 1);
	fseek ( diskFile , FATSTART_OFFSET , SEEK_SET );
	//fgets(buffer, FATBLOCKS_OFFSET - FATSTART_OFFSET + 1, diskFile);
	fread(buffer, sizeof(unsigned char), FATBLOCKS_OFFSET - FATSTART_OFFSET, diskFile);
	int fatStart = charArrayToInt(buffer);
	
	return fatStart;
}

int readFATBlocks(FILE *diskFile)
{
	unsigned char* buffer = malloc(ROOTDIRSTART_OFFSET - FATBLOCKS_OFFSET + 1);
	fseek ( diskFile , FATBLOCKS_OFFSET , SEEK_SET );
	//fgets(buffer, ROOTDIRSTART_OFFSET - FATBLOCKS_OFFSET + 1, diskFile);
	fread(buffer, sizeof(unsigned char), ROOTDIRSTART_OFFSET - FATBLOCKS_OFFSET, diskFile);
	int fatBlocks = charArrayToInt(buffer);
	return fatBlocks;
}

int readDirStart(FILE *diskFile)
{
	unsigned char* buffer = malloc(ROOTDIRBLOCKS_OFFSET - ROOTDIRSTART_OFFSET + 1);
	fseek ( diskFile , ROOTDIRSTART_OFFSET , SEEK_SET );
	//fgets(buffer, ROOTDIRBLOCKS_OFFSET - ROOTDIRSTART_OFFSET + 1, diskFile);
	fread(buffer, sizeof(unsigned char), ROOTDIRBLOCKS_OFFSET - ROOTDIRSTART_OFFSET, diskFile);
	int dirStart = charArrayToInt(buffer);
	return dirStart;
}

int readDirBlocks(FILE *diskFile)
{
	unsigned char* buffer = malloc(4 + 1);
	fseek ( diskFile , ROOTDIRBLOCKS_OFFSET , SEEK_SET );
	//fgets(buffer, ROOTDIRBLOCKS_OFFSET + 4 + 1, diskFile);
	fread(buffer, sizeof(unsigned char), ROOTDIRBLOCKS_OFFSET + 4, diskFile);
	int dirBlocks = charArrayToInt(buffer);
	return dirBlocks;
}

void findFATBlocks(FILE *diskFile, int fatStart, int numFATBlocks, int blockSize)
{
	unsigned char* buffer = malloc(FAT_ENTRY_SIZE + 1);
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

void getFileInfo(FILE *diskFile, int rootDirStart, int numRootDirBlocks, int blockSize)
{
	unsigned char* buffer = malloc(DIRECTORY_ENTRY_SIZE + 1);
	fseek ( diskFile , (rootDirStart * blockSize) , SEEK_SET );
	int dirPos;
	for(dirPos = 0; dirPos < numRootDirBlocks * blockSize; dirPos += DIRECTORY_ENTRY_SIZE)
	{
		//fgets(buffer, DIRECTORY_ENTRY_SIZE + 1, diskFile);
		fread(buffer, sizeof(unsigned char), DIRECTORY_ENTRY_SIZE, diskFile);
		char status = buffer[0]; // Declaring variable for code readablilty
		char dataType = 0;
		if((status & 1) == 0)
		{
			// Directory entry is free
			dataType = 0;
		}
		else
		{
			if((status & 2) == 2)
			{
				dataType = 'F';
			}
			else if((status & 4) == 4)
			{
				dataType = 'D';
			}
		}

		if(dataType != 0)
		{
			int fileSize = charArrayToInt(&buffer[9]); // Passes in a address that points to the fileSize portion of the directory entry
			char* fileSizeBuffer = calloc(11, sizeof(char));
			char* arrFileSize = calloc(11, sizeof(char));
			snprintf(fileSizeBuffer, 11, "%u", fileSize);
			int bufferTracker = 10;
			int arrayTracker = 9;

			for(; bufferTracker >= 0; bufferTracker--)
			{
				arrFileSize[bufferTracker] = ' ';
				if(fileSizeBuffer[bufferTracker] != 0)
				{
					arrFileSize[arrayTracker] = fileSizeBuffer[bufferTracker];
					arrayTracker--;
				}
			}

			char* fileNameBuffer = calloc(31, sizeof(char));
			int fileNameTracker = 30;
			for(bufferTracker = 30; bufferTracker >= 0; bufferTracker--)
			{
				fileNameBuffer[bufferTracker] = ' ';
				if(buffer[bufferTracker + 27] != 0)
				{
					fileNameBuffer[fileNameTracker] = buffer[bufferTracker + 27];
					fileNameTracker--;
				}
			}

			int modifedYear = (buffer[20] << 8) + buffer[21];
			//printf("%x, %x, %x\n", buffer[24], buffer[25], buffer[26]);
			
			printf("%c ", dataType);
			printf(arrFileSize);
			printf(" ");
			printf(fileNameBuffer);
			printf(" ");
			printf("%04d/%02d/%02d %02d:%02d:%02d", modifedYear, buffer[22], buffer[23], buffer[24], buffer[25], buffer[26]);
			printf("\n");
		}
	}
}