/*************************************************************************************//**
		* File: go.h
		* Description: name of main functions in GC. Walking through the objects and deleting the objects.
		* Update: 17/10/13
*****************************************************************************************/

#pragma once

/**
* @function go
* @detailed recursive traversing function (implements mark gc phase);
	marks object as alive and folloeing its object meta,
	starting from pointer "v", it walk around reached objects graph
	by calling itself (function go) for all objects object "v" points to;
* @return nothing
* @param v --- is a current traversing object (in first call --- roots and fake roots)
*/
void go (void *v);

/**
* @function mark_and_sweep
* @detailed implements mark and sweep stop the world algorithm
*/
void mark_and_sweep ();

/**
* @function gc_delete
* @detailed gc delete function
* @param chunk --- pointer on chunk to be freed
*/
/**
* @function gc
* @detailed forse garbage collection call for malloc's from msmalloc
* @return 0 in normal case; 1 in unsafe point case (nesting_level != 0)
*/
#ifndef HEADER_FILE
#define HEADER_FILE

#ifdef __cplusplus
	extern "C" {
#endif
		int gc ();
		void gc_delete (void *);
#ifdef __cplusplus
	}
#endif

#endif

/**
* @function get_next_obj
* @return pointer (void *) on the object on that root or gc_ptr "v" points;
	in case v is invalide pointer to the gc_ptr "v" returns NULL.
* @param v --- pointer (like gc_ptr)
*/
inline void * get_next_obj (void * v);