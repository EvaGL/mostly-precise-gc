/*************************************************************************************//**
		* File: gc_new.h
		* Description: This file consists memory allocation primitive gc_new and
			object meta classes realisations.
*****************************************************************************************/

#pragma once
#include <cstdio>
#include <pthread.h>
#include "go.h"
#include "meta_information.h"
#include <vector>
#include <assert.h>
#include "gc_ptr.h"
#include "debug_print.h"
#include "threading.h"
#include "tlvars.h"

/**
* @function gc_new
* @brief library memory allocation primitive
* @detailed allocates memory in program heap for managed object
* @return smart pointer gc_ptr (pointes to managed memory)
* @param types arguments for non-default constructor call
* @param count is an array size (defailt = 1 --- single object)
* @param T is an allocating object type
* @param Types --- arguments types of non-default constructor arguments
*/
template <class T, typename ... Types>
gc_ptr<T> gc_new (Types ... types, size_t count = 1) {
	assert(count >= 0);
	pthread_mutex_lock(&gc_mutex);
	tlvars * new_obj_flags = get_thread_handler()->tlflags;
	pthread_mutex_unlock(&gc_mutex);
	if (new_obj_flags->nesting_level == 0) {
		safepoint();
	}
	dprintf("in gc_new: \n");
	// get pointer to class meta or NULL if it is no meta for this class
	size_t * clMeta = get_meta<T>();
	dprintf("\tclMeta=%p\n", clMeta);

	/* set global active flags */
	bool old_new_active = new_obj_flags->new_active;
	new_obj_flags->new_active = true;  /* set flag that object creates(allocates) in heap */
	bool old_no_active = new_obj_flags->no_active;
	/* clMeta != NULL => no_active == true; false --- otherwise */
	new_obj_flags->no_active = clMeta != nullptr;
	/* metainformation that will be stored directly with object */
	meta<T>* m_inf = NULL;
	/* initialize object which will be store the result and allocate space */
	void * res = my_malloc(sizeof(T) * count + sizeof(meta<T>));
	dprintf("gc_new: %p\n", res);
	new_obj_flags->nesting_level++;

	if (clMeta != NULL) {
		dprintf("\tclMeta != NULL\n");
		if (count == 1) {
			dprintf("\tcount == 1\n");
			new ((char *)res + sizeof(meta<T>)) T(types ... );  /* create object in allocated space, call gc_ptr constructor, get new struct offsets */
		} else {
			dprintf("\tcount != 1\n");
			new ((char *)res + sizeof(meta<T>)) T[count];
		}
	} else {
	/* in this case we might to create clMeta (class meta information) and save in in ClassMetaList */
		dprintf("\tclMeta == NULL\n");
		/* save old offsets */
		std::vector<size_t> temp;
		temp.swap(new_obj_flags->offsets);
		/* save current pointer to the allocating object; it uses in gc_ptr constructor to evaluate offsets size */
		size_t old_current_pointer_to_object = new_obj_flags->current_pointer_to_object;
		new_obj_flags->current_pointer_to_object = reinterpret_cast <size_t> (res + sizeof(meta<T>));

		if (count == 1) {
			dprintf("\tcount == 1\n");
			new ((char *)res + sizeof(meta<T>)) T(types ... );  /* create object in allocated space, call gc_ptr constructor, get new struct offsets */
		} else {
			dprintf("\tcount != 1\n");
			new ((char*) res + sizeof(meta<T>)) T();
			new_obj_flags->no_active = true;
			new ((char *)res + sizeof(meta<T>) + sizeof(T)) T[count - 1];
		}
		clMeta = create_meta<T>(new_obj_flags);
		/* restore old global variable values */
		new_obj_flags->offsets.swap(temp);
		new_obj_flags->current_pointer_to_object = old_current_pointer_to_object;
	}
	dprintf("\tgc_new: will restore global values\n");
	m_inf = reinterpret_cast <meta<T>* > (res); /* stored pointer on meta */
	m_inf->shell = clMeta;
	m_inf->count = count;

	/* restore old global variable values */
	new_obj_flags->new_active = old_new_active;
	new_obj_flags->nesting_level--;
	new_obj_flags->no_active = old_no_active;

	dprintf("\tgc_new : before remove\n");
	/*return ptr on allocated space, begining value */
	return gc_ptr<T>((T*)((char *)res + sizeof(meta<T>)));
}