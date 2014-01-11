#include "meta_information.h"
#include <sys/mman.h>
#include <assert.h>
#include <string.h>

struct ClassesNamesPull {
    void * pullBegin;
    void * pullEnd;
    void * endOfMappedSpace;
    size_t length;

    ClassesNamesPull () {
        length = 4096;
        pullBegin = mmap(0, length, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        assert(pullBegin != MAP_FAILED);
        endOfMappedSpace = (void *)((char *)pullBegin + length / sizeof(char));
        pullEnd = pullBegin;
    }

    void allocateMoreSpace () {
        endOfMappedSpace = mmap(endOfMappedSpace, length, PROT_WRITE | PROT_READ, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        assert(endOfMappedSpace != MAP_FAILED);
        endOfMappedSpace = (void *)((char *)endOfMappedSpace + length / sizeof(char));
    }

    void * addClassName (const char * str) {
		if (endOfMappedSpace < (void *)((char *)pullEnd + strlen(str) / sizeof(char))) {
			endOfMappedSpace = mmap(endOfMappedSpace, length, PROT_WRITE | PROT_READ, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    	}
        void * result = pullEnd;
        strcpy((char *)pullEnd, str);
        pullEnd = (void *)((char *)pullEnd + strlen(str) / sizeof(char));
        return result;
    }
};
ClassesNamesPull classNamesPull;

MetaInformation * classMeta = NULL;


int contains (MetaInformation * meta, const char * str) {
    MetaInformation * temp = meta;
    while (temp != NULL) {
        if (memcmp(&(temp->name), str, strlen(str))) {
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}

void * getClassMetaPointer (MetaInformation * meta, const char * str) {
    MetaInformation * temp = meta;
    while (temp != NULL) {
        if (memcmp(&(temp->name), str, strlen(str))) {
            return temp->pointer;
        }
        temp = temp->next;
    }
    return NULL;
}

void addNewClassMetaInformation (const char * str, void * ptr) {
    MetaInformation * newMeta = (MetaInformation *) mmap(0, sizeof(struct MetaInformation),
        PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(newMeta != MAP_FAILED);
    newMeta->name = classNamesPull.addClassName(str);
    newMeta->pointer = ptr;
    newMeta->next = classMeta;
    classMeta = newMeta;
}