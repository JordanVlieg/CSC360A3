#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Constants.h"
#include "disk.h"

int charArrayToInt(unsigned char theBuffer[])
{
	int returnVal = (theBuffer[0] << 24) + (theBuffer[1] << 16) + (theBuffer[2] << 8) + theBuffer[3];
	return returnVal;
}

unsigned char* intToCharArray(int number)
{
	static unsigned char returnVals[4];
	returnVals[0] = ((number >> 24) & 0xFF);
	returnVals[1] = ((number >> 16) & 0xFF);
	returnVals[2] = ((number >> 8) & 0xFF);
	returnVals[3] = (number & 0xFF);
	return returnVals;
}

void setIntToBuffer(int number, unsigned char* buffer)
{
	buffer[0] = intToCharArray(number)[0];
	buffer[1] = intToCharArray(number)[1];
	buffer[2] = intToCharArray(number)[2];
	buffer[3] = intToCharArray(number)[3];
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

int* findFATBlocks(FILE *diskFile, int fatStart, int numFATBlocks, int blockSize)
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
	static int returnVals[3];
	returnVals[0] = available;
	returnVals[1] = reserved;
	returnVals[2] = allocated;
	
	return returnVals;
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

			//int startBlock = charArrayToInt(startingBlockPtr);
			
			printf("%c ", status);
			printf("%10u ", fileSize);
			printf("%30s ", fileNamePtr);
			printf("%04d/%02d/%02d %02d:%02d:%02d", modifedYear, buffer[22], buffer[23], buffer[24], buffer[25], buffer[26]);
			printf("\n");
		}
	}
	return;
}

void getFileFromClient(char diskFileName[], char hostFileName[], int rootDirStart, int numRootDirBlocks, int blockSize)
{
	FILE* diskFile;
	FILE* hostFile;
	diskFile = fopen(diskFileName,"r");

	unsigned char directoryBuffer[DIRECTORY_ENTRY_SIZE + 1];

	unsigned char* startingBlockPtr = &directoryBuffer[1];
	//unsigned char* numBlocksPtr = &directoryBuffer[5];
	unsigned char* fileSizePtr = &directoryBuffer[9];
	//unsigned char* createTimePtr = &directoryBuffer[13];
	//unsigned char* modifyTimePtr = &directoryBuffer[20];
	char* fileNamePtr = &directoryBuffer[27];
	//unsigned char* unusedPtr = &directoryBuffer[58];

	//int fileSize = -1;
	int startingFileBlock = -1;
	//int numFileBlocks = charArrayToInt(numBlocksPtr);

	//fseek ( diskFile , (startingFileBlock * blockSize) , SEEK_SET );

	fseek(diskFile, (rootDirStart * blockSize), SEEK_SET);

	fread(directoryBuffer, sizeof(unsigned char), DIRECTORY_ENTRY_SIZE, diskFile);
	
	int fatEntry = 0;
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
				startingFileBlock = charArrayToInt(startingBlockPtr);
				fileFound = 1;
				break;
			}
		}
		fread(directoryBuffer, sizeof(unsigned char), DIRECTORY_ENTRY_SIZE, diskFile);
	}
	unsigned char fatEntryBuff[4];
	if(fileFound)
	{
		int theFileSize = charArrayToInt(fileSizePtr);
		hostFile = fopen(hostFileName, "w");
		fclose(hostFile);
		hostFile = fopen(hostFileName, "a");
		fatEntry = startingFileBlock;
		//printf("FAT ENTRY: %x\n", fatEntry);
		int remainingBytes = theFileSize;
		while(fatEntry != FAT_EOF)
		{
			fseek(diskFile, (fatEntry * blockSize), SEEK_SET);
			unsigned char fileBuffer[DEFAULT_BLOCK_SIZE];
			fread(fileBuffer, sizeof(unsigned char), DEFAULT_BLOCK_SIZE, diskFile);
			if(remainingBytes < DEFAULT_BLOCK_SIZE)
			{
				fwrite(fileBuffer, sizeof(unsigned char), remainingBytes, hostFile);
			}
			else
			{
				fwrite(fileBuffer, sizeof(unsigned char), DEFAULT_BLOCK_SIZE, hostFile);
			}
			
			fseek(diskFile, (512 + fatEntry * 4), SEEK_SET);
			fread(fatEntryBuff, sizeof(unsigned char), 4, diskFile);

			fatEntry = charArrayToInt(fatEntryBuff);
			//printf("FAT ENTRY: %x\n", fatEntry);
			//char* garbage = NULL;
			//scanf("%c", garbage);
			remainingBytes -= DEFAULT_BLOCK_SIZE;
		}
		fclose(hostFile);
	}
	else
	{
		printf("File not found.\n");
	}
}

void putFileOnClient(char diskFileName[], char hostFileName[], int availableBlocks, int rootDirStart, int numRootDirBlocks, int blockSize, int fatStart, int numFatBlocks)
{
	FILE* hostFile;
	hostFile = fopen(hostFileName,"r");
	fseek(hostFile, 0, SEEK_END);
	int hostFileSize = ftell(hostFile);
	fseek(hostFile, 0, SEEK_SET);

	int numBlocksRequired = 1 + ((hostFileSize - 1) / DEFAULT_BLOCK_SIZE); // Ceiling division
	unsigned int fatChain[numBlocksRequired + 1];
	fatChain[numBlocksRequired] = FAT_EOF;

	


	if(availableBlocks < numBlocksRequired)
	{
		printf("Not enough free blocks on disk.\n");
		fclose(hostFile);
		return;
	}

	FILE* diskFile;
	diskFile = fopen(diskFileName,"r+");

	unsigned char directoryBuffer[DIRECTORY_ENTRY_SIZE + 1];

	unsigned char* startingBlockPtr = &directoryBuffer[1];
	unsigned char* numBlocksPtr = &directoryBuffer[5];
	unsigned char* fileSizePtr = &directoryBuffer[9];
	unsigned char* createTimePtr = &directoryBuffer[13];
	unsigned char* modifyTimePtr = &directoryBuffer[20];
	char* fileNamePtr = &directoryBuffer[27];
	//unsigned char* unusedPtr = &directoryBuffer[58];

	//int startingFileBlock = 0;
	//int numFileBlocks = charArrayToInt(numBlocksPtr);

	//fseek ( diskFile , (startingFileBlock * blockSize) , SEEK_SET );
	unsigned char fatEntryBuff[4];
	int fatEntry = 0;

	fseek(diskFile, (fatStart * DEFAULT_BLOCK_SIZE), SEEK_SET);

	int remainingBlocksRequired = numBlocksRequired;
	int chainPos = 0;
	//Change this to iterate over the FAT blocks instead
	for(; remainingBlocksRequired > 0;)
	{
		fread(fatEntryBuff, sizeof(unsigned char), FAT_ENTRY_SIZE, diskFile);
		fatEntry = charArrayToInt(fatEntryBuff);
		if(fatEntry == FAT_FREE)
		{

			/*block to fatEntry (NOT FAT ENTRY CONTENT)*/
			//512 + fatEntry*4


			fatChain[chainPos] = ((ftell(diskFile) - FAT_ENTRY_SIZE)) - (DEFAULT_BLOCK_SIZE * fatStart);
			chainPos++;
			remainingBlocksRequired--;
		}
	}

	
	fseek ( diskFile , (rootDirStart * blockSize) , SEEK_SET );
	int dirPos;
	for(dirPos = 0; dirPos < numRootDirBlocks * blockSize; dirPos += DIRECTORY_ENTRY_SIZE)
	{
		//fgets(buffer, DIRECTORY_ENTRY_SIZE + 1, diskFile);
		fread(directoryBuffer, sizeof(unsigned char), DIRECTORY_ENTRY_SIZE, diskFile);

		char status = getStatusChar(directoryBuffer[0]);
		

		if(status == 0)
		{

			time_t rawtime;
			struct tm *theTime;

			time( &rawtime );

			theTime = localtime( &rawtime );

			createTimePtr[6] = (theTime->tm_sec & 255);
			createTimePtr[5] = (theTime->tm_min & 255);
			createTimePtr[4] = (theTime->tm_hour & 255);
			createTimePtr[3] = (theTime->tm_mday & 255);
			createTimePtr[2] = (theTime->tm_mon & 255) + 1;
			createTimePtr[1] = ((theTime->tm_year + 1900) & 255);
			createTimePtr[0] = (((theTime->tm_year + 1900) >> 8) & 255);

			modifyTimePtr[6] = (theTime->tm_sec & 255);
			modifyTimePtr[5] = (theTime->tm_min & 255);
			modifyTimePtr[4] = (theTime->tm_hour & 255);
			modifyTimePtr[3] = (theTime->tm_mday & 255);
			modifyTimePtr[2] = (theTime->tm_mon & 255) + 1;
			modifyTimePtr[1] = ((theTime->tm_year + 1900) & 255);
			modifyTimePtr[0] = (((theTime->tm_year + 1900) >> 8) & 255);

			//printf("Current local time and date: %s", asctime(info));



			directoryBuffer[0] = 3;
			setIntToBuffer((fatChain[0] / 4), startingBlockPtr);
			setIntToBuffer(numBlocksRequired, numBlocksPtr);
			setIntToBuffer(hostFileSize, fileSizePtr);

			strncpy(fileNamePtr, hostFileName, 31);
			directoryBuffer[58] = 0xFF;
			directoryBuffer[59] = 0xFF;
			directoryBuffer[60] = 0xFF;
			directoryBuffer[61] = 0xFF;
			directoryBuffer[62] = 0xFF;
			directoryBuffer[63] = 0xFF;

			int writeLoc = ftell(diskFile) - DIRECTORY_ENTRY_SIZE;
			fseek(diskFile, writeLoc, SEEK_SET);
			fwrite(directoryBuffer, sizeof(unsigned char), DIRECTORY_ENTRY_SIZE, diskFile);
			break;
		}
	}

	//printf("Blocks required: %d\n", numBlocksRequired);
	unsigned char blockBuffer[DEFAULT_BLOCK_SIZE];
	int block = 0;
	for(; block < numBlocksRequired; block++)
	{
		//printf("Blocks written: %d\n", block+1);
		fread(blockBuffer, sizeof(unsigned char), DEFAULT_BLOCK_SIZE, hostFile);
		fseek(diskFile, ((DEFAULT_BLOCK_SIZE * (fatChain[block]/4))), SEEK_SET);
		fwrite(blockBuffer, sizeof(unsigned char), DEFAULT_BLOCK_SIZE, diskFile);
		if(block+1 == numBlocksRequired)
		{
			fseek(diskFile, (fatChain[block] + DEFAULT_BLOCK_SIZE * fatStart), SEEK_SET);
			fwrite(intToCharArray(FAT_EOF), sizeof(unsigned char), FAT_ENTRY_SIZE, diskFile);
		}
		else
		{
			fseek(diskFile, (fatChain[block] + DEFAULT_BLOCK_SIZE * fatStart), SEEK_SET);
			fwrite(intToCharArray(fatChain[block+1] / 4), sizeof(unsigned char), FAT_ENTRY_SIZE, diskFile);
		}
		memset(blockBuffer, 0, DEFAULT_BLOCK_SIZE);
	}



	fclose(hostFile);
	fclose(diskFile);
	return;
}