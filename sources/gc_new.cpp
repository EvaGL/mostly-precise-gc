/*************************************************************************************//**
        * File: gc_new.cpp
        * Description: This file consists describing code with realiasation functions  in "gc_new.h"
*****************************************************************************************/

#include "gc_new.h"
#include "go.h"
#include <cstdio>
#include "stack.h"

StackMap stack_ptr = StackMap::create_StackMap_instance(); //!< stack of stack pointers
std::vector <size_t> offsets;
bool new_active = false; /* global flag. False -- out gc_new, true -- in gc_new*/
MetaInformation * list_meta_obj = new MetaInformation();
size_t counter = 0; /* stored count of occupated blocks */
int nesting_level = 0; /* gc_new nesting level */
bool no_active = false; /* flag that says gc_ptr not to add himself in roots or offsets */
size_t current_pointer_to_object = 0;