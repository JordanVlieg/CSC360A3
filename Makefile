# CC = gcc
CFLAGS = -Wall -g

DINFO = diskinfo.o disk.o
DLIST = disklist.o disk.o
DGET = diskget.o disk.o
DPUT = diskput.o disk.o

all: diskinfo disklist diskget diskput

diskinfo: $(DINFO)
	$(CXX) $(CFLAGS) -o diskinfo $(DINFO)

disklist: $(DLIST)
	$(CXX) $(CFLAGS) -o disklist $(DLIST)

diskget: $(DGET)
	$(CXX) $(CFLAGS) -o diskget $(DGET)

diskput: $(DPUT)
	$(CXX) $(CFLAGS) -o diskput $(DPUT)

clean: 
	rm -rf $(DINFO) $(DLIST) $(DGET) $(DPUT) diskinfo disklist diskget diskput