#include "meta_information.h"
#include <sys/mman.h>
#include <assert.h>
#include "debug_print.h"

MetaInformation * classMeta = NULL;

void * contains (MetaInformation * meta, const void * name) {
	dprintf("MetaInformation: contains: begin\n");
	MetaInformation * temp = meta;
	while (temp != NULL) {
		if (name == temp->name) {
			dprintf("MetaInformation: contains: yes: %p\n", temp->pointer);
			return temp->pointer;
		}
		temp = temp->next;
	}
	dprintf(" no class meta \t");
	return NULL;
}

void * getClassMetaPointer (MetaInformation * meta, const void * name){
	dprintf("getClassMetaPointer\n");
	return contains(meta, name);
}

void addNewClassMetaInformation	(const void * name, void * ptr){
	dprintf("addNewClassMetaInformation\n");
	MetaInformation * newMeta = (MetaInformation *) mmap(0, sizeof(struct MetaInformation),
		PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	assert(newMeta != MAP_FAILED);
	newMeta->name = name;
	newMeta->pointer = ptr;
	newMeta->next = classMeta;
	classMeta = newMeta;
}