/*************************************************************************************//**
        * File: gc_new.cpp
        * Description: This file consists some global data.
        TODO: global data might be disposed of ...
*****************************************************************************************/

#include "gc_new.h"
#include "go.h"
#include <cstdio>
#include "stack.h"

StackMap stack_ptr = StackMap::create_StackMap_instance();	//!< root stack
std::vector <size_t> offsets;	/// global data to store offsets from the class
								/// constructs in gc_ptr;
								/// releases and transforms to class meta in gc_new.
bool new_active = false; /* global flag. False -- gc_new is unactive, true -- gc_new is active*/
MetaInformation * list_meta_obj = new MetaInformation();	/// global list with class meta
/// TODO: counter might be releazed
size_t counter = 0;	/* stored count of occupated blocks */
int nesting_level = 0;	/* gc_new nesting level (recursion depth) */
bool no_active = false;	/* flag that says gc_ptr might not to add himself in roots or offsets */
size_t current_pointer_to_object = 0;