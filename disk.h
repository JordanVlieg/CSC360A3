#ifndef CHECKER
#define CHECKER

int charArrayToInt(unsigned char* theBuffer);

//char* readFSID(FILE *diskFile);

int readBlockSize(FILE *diskFile);

int readBlockCount(FILE *diskFile);

int readFATStart(FILE *diskFile);

int readFATBlocks(FILE *diskFile);

int readDirStart(FILE *diskFile);

int readDirBlocks(FILE *diskFile);

void findFATBlocks(FILE *diskFile, int fatStart, int numFATBlocks, int blockSize);

char getStatusChar(char statusByte);

void getFileInfo(FILE *diskFile, int rootDirStart, int numRootDirBlocks, int blockSize);

void getFileFromClient(FILE *diskFile, FILE *hostFile, char hostFileName[], int rootDirStart, int numRootDirBlocks, int blockSize);

#endif