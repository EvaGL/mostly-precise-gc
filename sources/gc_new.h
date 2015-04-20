/*************************************************************************************//**
		* File: gc_new.h
		* Description: This file consists memory allocation primitive gc_new and
			object meta classes realisations.
*****************************************************************************************/

#pragma once
#include <cstdio>
#include <pthread.h>
#include "go.h"
#include "taginfo.h"
#include <typeinfo>
#include "meta_information.h"
#include <vector>
#include <assert.h>
#include "gc_ptr.h"
#include "debug_print.h"
#include "threading.h"

extern thread_local std::vector <size_t> offsets;
extern thread_local bool new_active;
extern thread_local bool no_active;
extern thread_local size_t counter;
extern thread_local MetaInformation * classMeta;
extern thread_local int nesting_level;
extern thread_local size_t current_pointer_to_object;

/**
* @function hasOffsets
* @brief checks has class T some gc_ptr in or not
* @detailed checks has class T some gc_ptr in or not;
*	it also constructs class-meta-information for class T
*	iff this meta-information isn't exists yet;
*	P.S. it calls in gc_new in case clMeta(class-meta-information) == NULL
*		and object that is allocating is an array (count != 1);
*		In this case we have no meta-information for class T
*		and to costruct it hasOffsets allocates automatic single object
*		typeof T with help of calling default contructor of T class;
* @return bool: true --- calss T has some offsets, false --- otherwise
*/
template <class T>
bool hasOffsets (void) {
	dprintf("in hasOffsets\n");
	nesting_level++;
	/* save global variable values*/
	std::vector<size_t> temp;
	temp.swap(offsets);
	size_t old_current_pointer_to_object = current_pointer_to_object;
	void * type_name_pointer = (void*)typeid(T).name();
	void * clMeta = contains(classMeta, type_name_pointer);
	/* allocate space */
	void * res = my_malloc(sizeof(T) + sizeof(void*) + sizeof(meta<T>));
	current_pointer_to_object = reinterpret_cast <size_t> (res + sizeof(void*) + sizeof(meta<T>));
	new ((char *)res + sizeof(void*) + sizeof(meta<T>)) T();
	*((size_t*)((char *)res + sizeof(meta<T>))) =  reinterpret_cast <size_t> (new (res) meta<T>);  /* initialize meta in obj */
	meta<T>* m_inf = reinterpret_cast <meta<T>* > (res);  /* stored pointer on meta */

	if (offsets.empty()) {
		if (clMeta) {
			m_inf->shell = clMeta; 
		} else {
			/** create new type - box, save in shell;
			* args: first --- 0 --- no offsets,
			*		second --- 0 --- size,
			*		num_of_elements --- 1.
			* this args means, that it is simple object without any offsets.
			*/
			m_inf->shell = create_generic_object(0, 0, 1);
			addNewClassMetaInformation(type_name_pointer, m_inf->shell);
		}
	} else {
		if (clMeta) {
			m_inf->shell = clMeta;
		} else {
			m_inf->shell = generic_box_struct (std::move(offsets), sizeof(T), 1);
			addNewClassMetaInformation(type_name_pointer, m_inf->shell);
		}
	}

	bool result = (!offsets.empty());
	/* restore old global variable values */
	temp.swap(offsets);
	temp.clear();
	current_pointer_to_object = old_current_pointer_to_object;
	nesting_level--;
	dprintf("hasOffsets: return\n");
	return result;
}

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
	if (nesting_level == 0) {
		safepoint();
	}
	dprintf("in gc_new: \n");
	void * type_name_pointer = (void*)typeid(T).name();
	// get pointer to class meta or NULL if it is no meta for this class
	void * clMeta = contains(classMeta, type_name_pointer);
	dprintf("\tclMeta=%p\n", clMeta);

	/* set global active flags */
	bool old_new_active = new_active;
	new_active = true;  /* set flag that object creates(allocates) in heap */
	bool old_no_active = no_active;
	/* clMeta != NULL => no_active == true; false --- otherwise */
	no_active = clMeta;
	/* metainformation that will be stored directly with object */
	meta<T>* m_inf = NULL;
	/* initialize object which will be store the result and allocate space */
	void * res = my_malloc(sizeof(T) * count + sizeof(meta<T>));
	dprintf("gc_new: %p\n", res);
	nesting_level++;

	if (clMeta != NULL) {
		dprintf("\tclMeta != NULL\n");
		if (count == 1) {
			dprintf("\tcount == 1\n");
			new ((char *)res + sizeof(meta<T>)) T(types ... );  /* create object in allocated space, call gc_ptr constructor, get new struct offsets */
			m_inf = reinterpret_cast <meta<T>* > (res); /* stored pointer on meta */
			m_inf->shell = clMeta;
		} else {
			dprintf("\tcount != 1\n");
			no_active = true; // offsets wont counting yet
			new ((char *)res + sizeof(void *) + sizeof(meta<T>)) T[count];
			m_inf = reinterpret_cast <meta<T>* > (res);

			no_active = true; // offsets will counting yet
			new_active = true; // restore, because string new ... T[coint]; changed it to false
			BLOCK_TAG* tag = (BLOCK_TAG *) clMeta; /* get meta tag */
			size_t offsets_count = 0;

			dprintf("\ttag Model: %i\n", tag->model);
			try {
				switch (tag->model) {
					case 1: {  /* boxed object */
						offsets_count = *(size_t *)((char *)clMeta + sizeof(BLOCK_TAG));  /* count of offsets*/
					}
					break;
					case 2: /* simple obj */
						break;
					case 3: { /* boxed array */
							offsets_count = *(size_t *)((char *)tag->ptr + sizeof(BLOCK_TAG));
						}
						break;
					case 4: /* unboxed_array */
						break;
					default:
						throw tag;
						break;
				}
			} catch(BLOCK_TAG* tag) {
				printf("FUNCTION gc_new : tag : catch1 : out of memory\n"); fflush(stdout);
				exit(1);
			} catch(...) {
				printf("gc_new:\nUNEXPECTED ERROR!!! CHECK tag->mbit");	fflush(stdout);
				exit(1);
			}
			if (offsets_count != 0) {
				dprintf("\toffset count != 0\n");
				m_inf->shell = create_boxed_array(count, clMeta, sizeof(T));
			} else {
				dprintf("\toffset count == 0\n");
				m_inf->shell = create_unboxed_array(count);
			}
		}

	} else {
	/* in this case we might to create clMeta (class meta information) and save in in ClassMetaList */
		dprintf("\tclMeta == NULL\n");
		/* save old offsets */
		std::vector<size_t> temp;
		temp.swap(offsets);
		/* save current pointer to the allocating object; it uses in gc_ptr constructor to evaluate offsets size */
		size_t old_current_pointer_to_object = current_pointer_to_object;
		current_pointer_to_object = reinterpret_cast <size_t> (res + sizeof(meta<T>));

		if (count == 1) {
			dprintf("\tcount == 1\n");
			new ((char *)res + sizeof(meta<T>)) T(types ... );  /* create object in allocated space, call gc_ptr constructor, get new struct offsets */
			m_inf = reinterpret_cast <meta<T>* > (res);  /* stored pointer on meta */

			if (offsets.empty()) {
				dprintf("\toffsets is empty\n");
				m_inf->shell = create_generic_object(0, 0, 1); /* create new type - box, save in shell */
			} else {
				dprintf("\toffsets is not empty: count == %i\n", offsets.size());
				/*create new box and save pointer in shell */
				m_inf->shell = generic_box_struct (std::move(offsets), sizeof(T), count);
			}
			addNewClassMetaInformation(type_name_pointer, m_inf->shell);
		} else {
			dprintf("\tcount != 1\n");
			no_active = true; // offsets wont counting yet
			new ((char *)res + sizeof(meta<T>)) T[count];
			m_inf = reinterpret_cast <meta<T>* > (res);

			no_active = false; // offsets will counting yet
			new_active = true; // restore, because string new ... T[coint]; changed it to false
			dprintf("\tcall hasOffsets\n");
			// force counting class metainfo, because it can be that we have not meta for class T
			if (hasOffsets<T>()) {
				clMeta = contains(classMeta, type_name_pointer);
				assert(clMeta != NULL);
				m_inf->shell = create_boxed_array(count, clMeta, sizeof(T));
			} else {
				clMeta = contains(classMeta, type_name_pointer);
				m_inf->shell = create_unboxed_array(count);
			}
			assert(clMeta != NULL);
		}

		/* restore old global variable values */
		offsets.swap(temp);
		current_pointer_to_object = old_current_pointer_to_object;
	}
	dprintf("\tgc_new: will restore global values\n");
	m_inf->size = count; /* count of offsets */

	/* restore old global variable values */
	new_active = old_new_active;
	nesting_level--;
	no_active = old_no_active;

	dprintf("\tgc_new : before remove\n");
	/*return ptr on allocated space, begining value */
	return gc_ptr<T>((T*)((char *)res + sizeof(meta<T>)));
}