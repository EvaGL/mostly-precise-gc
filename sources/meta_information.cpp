#include "meta_information.h"
#include <sys/mman.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

struct ClassesNamesPull {
	char * pullBegin;
	char * pullEnd;
	char * endOfMappedSpace;
	size_t length;

	ClassesNamesPull () {
	#ifdef DEBUGE_MODE
		printf("ClassNamesPull\n"); fflush(stdout);
	#endif
		length = 4096;
		pullBegin = (char*)mmap(0, length, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		assert(pullBegin != MAP_FAILED);
		endOfMappedSpace = pullBegin + length * sizeof(char);
		pullEnd = pullBegin;
	}

	void allocateMoreSpace () {
	#ifdef DEBUGE_MODE
		printf("allocateMoreSpace\n"); fflush(stdout);
	#endif
		endOfMappedSpace = (char*)mmap(endOfMappedSpace, length, PROT_WRITE | PROT_READ, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		assert(endOfMappedSpace != MAP_FAILED);
		endOfMappedSpace = (endOfMappedSpace + length * sizeof(char));
	}

	char * addClassName (const char * str) {
	#ifdef DEBUGE_MODE
		printf("addClassMName\n"); fflush(stdout);
	#endif
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
#ifdef DEBUGE_MODE
	printf("contains\n"); fflush(stdout);
#endif
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
#ifdef DEBUGE_MODE
	printf("getClassMetaPointer\n"); fflush(stdout);
#endif
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
#ifdef DEBUGE_MODE
	printf("addNewClassMetaInformation\n"); fflush(stdout);
#endif
	MetaInformation * newMeta = (MetaInformation *) mmap(0, sizeof(struct MetaInformation),
		PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	assert(newMeta != MAP_FAILED);
	newMeta->name = classNamesPull.addClassName(str);
	newMeta->pointer = ptr;
	newMeta->next = classMeta;
	classMeta = newMeta;
}