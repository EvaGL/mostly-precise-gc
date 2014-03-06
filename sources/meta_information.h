#pragma once

struct MetaInformation {
    void * name; // hash key (type name)
    void * pointer; // pointer to class meta
    MetaInformation * next;
};

int contains (MetaInformation * meta, const char * str);
void * getClassMetaPointer (MetaInformation * meta, const char * str);
void addNewClassMetaInformation (const char * str, void * ptr);