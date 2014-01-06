all: output

output: malloc.o
		g++ malloc.o -std=c++11 ../object_repres/taginfo.cpp ../object_repres/boxing2.cpp ../object_repres/go.cpp ../Roots2/collect.cpp \
		../Roots2/stack.cpp ../gc_new/gc_new.cpp b.cpp -O0 -lpthread -o output

malloc.o: ../object_repres/malloc.c
		gcc -c ../object_repres/malloc.c

clean:
	rm -rf *o output