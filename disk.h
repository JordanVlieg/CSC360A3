#ifndef CHECKER
#define CHECKER

int charArrayToInt(unsigned char* theBuffer);

unsigned char* intToCharArray(int number);
//char* readFSID(FILE *diskFile);

int readBlockSize(FILE *diskFile);

int readBlockCount(FILE *diskFile);

int readFATStart(FILE *diskFile);

int readFATBlocks(FILE *diskFile);

int readDirStart(FILE *diskFile);

int readDirBlocks(FILE *diskFile);

int* findFATBlocks(FILE *diskFile, int fatStart, int numFATBlocks, int blockSize);

char getStatusChar(char statusByte);

void getFileInfo(FILE *diskFile, int rootDirStart, int numRootDirBlocks, int blockSize);

void getFileFromClient(char diskFileName[], char hostFileName[], int rootDirStart, int numRootDirBlocks, int blockSize);

void putFileOnClient(char diskFileName[], char hostFileName[], int availableBlocks, int rootDirStart, int numRootDirBlocks, int blockSize, int fatStart, int numFatBlocks);

#endif