SOURCE=sources/boxing2.cpp sources/collect.cpp sources/gc_new.cpp sources/go.cpp sources/stack.cpp sources/taginfo.cpp

all: gclib malloc
#	g++ -shared -o libgc.so bib/malloc.o *.o -lm
#	ls -G *.o bib/malloc.o -o libgc.so
	g++ -shared -o libgc.so *.o -lm
	rm -rf *.o

malloc:
	gcc -fPIC -c bib/malloc.c

gclib: $(SOURCE)
	g++ -std=c++11 -c -fPIC $(SOURCE)

clean:
	rm -rf *.o libgc.so