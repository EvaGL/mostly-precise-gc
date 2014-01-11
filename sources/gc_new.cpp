/*************************************************************************************//**
        * File: gc_new.cpp
        * Description: This file consists describing code with realiasation functions  in "gc_new.h"
        * Update: 20/10/13
*****************************************************************************************/

#include "gc_new.h"
#include "go.h"
#include <cstdio>

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;  /* create mutex for locking function of allocating */
PointerList * offsets = NULL;
bool new_active = false; /* global flag. False -- out gc_new, true -- in gc_new*/
MetaInformation * list_meta_obj = new MetaInformation();
size_t counter = 0; /* stored count of occupated blocks */