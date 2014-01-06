/*************************************************************************************//**
        * File: gc_new.cpp
        * Description: This file consists describing code with realiasation functions  in "gc_new.h"
        * Update: 20/10/13
*****************************************************************************************/

#include <map>
#include "gc_new.h"
#include "../object_repres/go.h"
#include <cstdio>
#include <string>

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;  /* create mutex for locking function of allocating */
//std::vector<void *> offsets; /* a stored pointers addreses in heap when new_active flag == true else its clear*/
PointerList * offsets = NULL;
bool new_active = false; /* global flag. False -- out gc_new, true -- in gc_new*/
//std::vector <void *> ptr_in_heap;  /* all pointers in heap */
//std::unordered_map <std::string, void *> list_meta_obj;  /* map objects metainfo */
MetaInformation * list_meta_obj = new MetaInformation();
size_t counter = 0; /* stored count of occupated blocks */
