SOURCE=sources/boxing2.cpp sources/collect.cpp sources/gc_new.cpp sources/go.cpp sources/stack.cpp sources/taginfo.cpp sources/meta_information.cpp sources/PointerList.cpp
CC=gcc
COMPIL_OPIONS=-shared -lm -o

all: gclib malloc
	$(CC) *.o $(COMPIL_OPIONS) libgc.so
#	rm -rf *.o

malloc: dlmalloc/malloc.c
	$(CC) -fPIC -c dlmalloc/malloc.c

gclib: $(SOURCE)
	$(CC) -std=c++11 -fPIC -c $(SOURCE)

clean:
	rm -rf libgc.so *.o