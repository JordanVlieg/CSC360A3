#A3OBJS = diskinfo.o disklist.o

#all: diskinfo disklist

#diskinfo: diskinfo.c
#	gcc -o diskinfo diskinfo.c

#disklist: disklist.c
#	gcc -o disklist disklist.c

#clean: 
#	rm -rf $(A3OBJS) diskinfo disklist



# CC = gcc
CFLAGS = -Wall -g

DINFO = diskinfo.o disk.o
DLIST = disklist.o disk.o

all: diskinfo disklist

diskinfo: $(DINFO)
	$(CXX) $(CFLAGS) -o diskinfo $(DINFO) $(LDFLAGS) 

disklist: $(DLIST)
	$(CXX) $(CFLAGS) -o disklist $(DLIST) $(LDFLAGS) 

clean: 
	rm -rf $(DINFO) $(DLIST) diskinfo disklist