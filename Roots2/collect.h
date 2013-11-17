/****************************************************************************************
	* File: collect.h
	* Description: Conteins all pointers list description. This pointers collects to ???
*****************************************************************************************/

#pragma once
#include <set>
#include <vector>
#include <utility>
#include <map>
#include <cassert>
#include "../gc_new/gc_new.h"
#include <cstdio>
#include "stack.cpp"

/**
* @struct ptr_list
* @brief Double-Linked list of pointers.
* @delailed Pointers container. Realizes double-linked list. Destinated to collect all reachable pointers
*/
struct ptr_list {  
	void *ptr; /**< a stored pointer */
	bool stack_ptr; /**< is this pointer a pointer in stack or not(True - pointer in the stack, False - pointer in the heap) */
	ptr_list *next; /**< pointer to the next list element */
	ptr_list *prev; /**< pointer to the previous list element */

	/**< a default constructor */
	ptr_list () :ptr(0), next(0), prev(0) { }

	/** 
	* @brief consrtuctor on pointer
	* @param v pointer to collect
	*/
	ptr_list (void *v) :ptr(v), next(0), prev(0) { }
};

/** 
* @brief add element in list
* @detailed add element ?where? list
* @param head list(list head pointer)
* @param ptr pointer to add
* @return updated pointers list
*/
ptr_list* add (ptr_list *&head, void *ptr);

/** 
* @brief deletes element from list
* @detailed deletes element ?out of where? list
* @param head list(list head pointer)
* @param me removing pointer
*/
void del (ptr_list *&head, ptr_list *me);

/** 
* @brief clear list
* @detailed deletes all element from list, i.e. free list
* @param v list(list head pointer) to be free
*/
void clear (ptr_list *v);

/** 
* @brief adds pointer 
* @detailed adds pointer to stack pointers list(declared in collect.cpp)
* @param p pointer to be added
* @return updated stack pointers list
*/
void inc (void *p);

/** 
* @brief deletes pointer
* @detailed deletes pointer from stack pointers list(declared in collect.cpp)
* @param me removing pointer
*/
void dec ();
