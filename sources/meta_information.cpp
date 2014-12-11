#include "meta_information.h"
#include <sys/mman.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "debug_print.h"

struct ClassesNamesPull {
	char * pullBegin;
	char * pullEnd;
	char * endOfMappedSpace;
	size_t length;

	ClassesNamesPull () {
		dprintf("ClassNamesPull\n");
		length = 4096;
		pullBegin = (char*)mmap(0, length, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		assert(pullBegin != MAP_FAILED);
		endOfMappedSpace = pullBegin + length * sizeof(char);
		pullEnd = pullBegin;
	}

	void allocateMoreSpace () {
		dprintf("allocateMoreSpace\n");
		endOfMappedSpace = (char*)mmap(endOfMappedSpace, length, PROT_WRITE | PROT_READ, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		assert(endOfMappedSpace != MAP_FAILED);
		endOfMappedSpace = (endOfMappedSpace + length * sizeof(char));
	}

	char * addClassName (const char * str) {
		dprintf("addClassMName\n");
		if (endOfMappedSpace < (pullEnd + (strlen(str) + 1) * sizeof(char))) {
			endOfMappedSpace = (char*)mmap(endOfMappedSpace, length, PROT_WRITE | PROT_READ, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		}
		char * result = pullEnd;
		strcpy(pullEnd, str);
		strcpy(pullEnd + (strlen(str)), "\0");
		pullEnd = (pullEnd + (strlen(str) + 1) * sizeof(char));
		return result;
	}
};
ClassesNamesPull classNamesPull;

MetaInformation * classMeta = NULL;

void * contains (MetaInformation * meta, const char * str) {
	dprintf("contains\n");
	MetaInformation * temp = meta;
	while (temp != NULL) {
		char * metaName = temp->name;
		int i = 0, len = strlen(str);
		for (; i < strlen(str); i++) {
			if (metaName[i] != str[i]) {
				break;
			}
		}
		if (i == len && metaName[i] == '\0') {
			return temp->pointer;
		}
		temp = temp->next;
	}
#ifdef DEBUGE_MODE
	printf(" no class meta \t");
	for (int i = 0; i < strlen(str); i++) {
		printf("%c", str[i]);
	}
	printf("\n");
#endif
	return NULL;
}

void * getClassMetaPointer (MetaInformation * meta, const char * str) {
	dprintf("getClassMetaPointer\n");
	MetaInformation * temp = meta;
	while (temp != NULL) {
		char * metaName = (temp->name);
		int i = 0, len = strlen(str);
		for (; i < strlen(str); i++) {
			if (metaName[i] != str[i]) {
				break;
			}
		}
		if (i == len && metaName[i] == '\0') {
			return temp->pointer;
		}
		temp = temp->next;
	}
	return NULL;
}

void addNewClassMetaInformation (const char * str, void * ptr) {
	dprintf("addNewClassMetaInformation\n");
	MetaInformation * newMeta = (MetaInformation *) mmap(0, sizeof(struct MetaInformation),
		PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	assert(newMeta != MAP_FAILED);
	newMeta->name = classNamesPull.addClassName(str);
	newMeta->pointer = ptr;
	newMeta->next = classMeta;
	classMeta = newMeta;
}