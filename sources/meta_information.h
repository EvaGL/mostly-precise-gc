#pragma once

struct MetaInformation {
	char * name; // hash key (type name)
	void * pointer; // pointer to class meta
	MetaInformation * next;
};

void * contains					(MetaInformation * meta, const char * str);
void * getClassMetaPointer		(MetaInformation * meta, const char * str);
void addNewClassMetaInformation	(const char * str, void * ptr);