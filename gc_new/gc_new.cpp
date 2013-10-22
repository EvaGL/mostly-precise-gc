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
std::vector <void *> offsets;  /* struct offsets - stored offsets */ 
std::vector <void *> ptr_in_heap;  /* all ptrs in heap */
std::unordered_map <std::string, void *> list_meta_obj;  /* map objects metainfo */
bool new_active = false;  /* where we creating object(true - heap) */
size_t counter = 0; /* stored count of occupated blocks */
