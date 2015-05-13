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
#include "tlvars.h"

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
bool hasOffsets (tlvars * new_obj_flags) {
	dprintf("in hasOffsets\n");
	new_obj_flags->nesting_level++;
	/* save global variable values*/
	std::vector<size_t> temp;
	temp.swap(new_obj_flags->offsets);
	size_t old_current_pointer_to_object = new_obj_flags->current_pointer_to_object;
	void * type_name_pointer = (void*)typeid(T).hash_code();
	void * clMeta = contains(new_obj_flags->classMeta, type_name_pointer);
	/* allocate space */
	void * res = my_malloc(sizeof(T) + sizeof(void*) + sizeof(meta<T>));
	new_obj_flags->current_pointer_to_object = reinterpret_cast <size_t> (res + sizeof(void*) + sizeof(meta<T>));
	new ((char *)res + sizeof(void*) + sizeof(meta<T>)) T();
	*((size_t*)((char *)res + sizeof(meta<T>))) =  reinterpret_cast <size_t> (new (res) meta<T>);  /* initialize meta in obj */
	meta<T>* m_inf = reinterpret_cast <meta<T>* > (res);  /* stored pointer on meta */

	if (new_obj_flags->offsets.empty()) {
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
			addNewClassMetaInformation(new_obj_flags, type_name_pointer, m_inf->shell);
		}
	} else {
		if (clMeta) {
			m_inf->shell = clMeta;
		} else {
			m_inf->shell = generic_box_struct (std::move(new_obj_flags->offsets), sizeof(T), 1);
			addNewClassMetaInformation(new_obj_flags, type_name_pointer, m_inf->shell);
		}
	}

	bool result = (!new_obj_flags->offsets.empty());
	/* restore old global variable values */
	temp.swap(new_obj_flags->offsets);
	temp.clear();
	new_obj_flags->current_pointer_to_object = old_current_pointer_to_object;
	new_obj_flags->nesting_level--;
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
	pthread_mutex_lock(&gc_mutex);
	tlvars * new_obj_flags = get_thread_handler(pthread_self())->tlflags;
	pthread_mutex_unlock(&gc_mutex);
	if (new_obj_flags->nesting_level == 0) {
		safepoint();
	}
	dprintf("in gc_new: \n");
	void * type_name_pointer = (void*)typeid(T).hash_code();
	// get pointer to class meta or NULL if it is no meta for this class
	void * clMeta = contains(new_obj_flags->classMeta, type_name_pointer);
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
	void * res = malloc(sizeof(T) * count + sizeof(meta<T>));
	dprintf("gc_new: %p\n", res);
	new_obj_flags->nesting_level++;

	if (clMeta != NULL) {
		dprintf("\tclMeta != NULL\n");
		if (count == 1) {
			dprintf("\tcount == 1\n");
			new ((char *)res + sizeof(meta<T>)) T(types ... );  /* create object in allocated space, call gc_ptr constructor, get new struct offsets */
			m_inf = reinterpret_cast <meta<T>* > (res); /* stored pointer on meta */
			m_inf->shell = clMeta;
		} else {
			dprintf("\tcount != 1\n");
			new_obj_flags->no_active = true; // offsets wont counting yet
			new ((char *)res + sizeof(void *) + sizeof(meta<T>)) T[count];
			m_inf = reinterpret_cast <meta<T>* > (res);

			new_obj_flags->no_active = true; // offsets will counting yet
			new_obj_flags->new_active = true; // restore, because string new ... T[coint]; changed it to false
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
		temp.swap(new_obj_flags->offsets);
		/* save current pointer to the allocating object; it uses in gc_ptr constructor to evaluate offsets size */
		size_t old_current_pointer_to_object = new_obj_flags->current_pointer_to_object;
		new_obj_flags->current_pointer_to_object = reinterpret_cast <size_t> (res + sizeof(meta<T>));

		if (count == 1) {
			dprintf("\tcount == 1\n");
			new ((char *)res + sizeof(meta<T>)) T(types ... );  /* create object in allocated space, call gc_ptr constructor, get new struct offsets */
			m_inf = reinterpret_cast <meta<T>* > (res);  /* stored pointer on meta */

			if (new_obj_flags->offsets.empty()) {
				dprintf("\toffsets is empty\n");
				m_inf->shell = create_generic_object(0, 0, 1); /* create new type - box, save in shell */
			} else {
				dprintf("\toffsets is not empty: count == %i\n", offsets.size());
				/*create new box and save pointer in shell */
				m_inf->shell = generic_box_struct (std::move(new_obj_flags->offsets), sizeof(T), count);
			}
			addNewClassMetaInformation(new_obj_flags, type_name_pointer, m_inf->shell);
		} else {
			dprintf("\tcount != 1\n");
			new_obj_flags->no_active = true; // offsets wont counting yet
			new ((char *)res + sizeof(meta<T>)) T[count];
			m_inf = reinterpret_cast <meta<T>* > (res);

			new_obj_flags->no_active = false; // offsets will counting yet
			new_obj_flags->new_active = true; // restore, because string new ... T[coint]; changed it to false
			dprintf("\tcall hasOffsets\n");
			// force counting class metainfo, because it can be that we have not meta for class T
			if (hasOffsets<T>(new_obj_flags)) {
				clMeta = contains(new_obj_flags->classMeta, type_name_pointer);
				assert(clMeta != NULL);
				m_inf->shell = create_boxed_array(count, clMeta, sizeof(T));
			} else {
				clMeta = contains(new_obj_flags->classMeta, type_name_pointer);
				m_inf->shell = create_unboxed_array(count);
			}
			assert(clMeta != NULL);
		}

		/* restore old global variable values */
		new_obj_flags->offsets.swap(temp);
		new_obj_flags->current_pointer_to_object = old_current_pointer_to_object;
	}
	dprintf("\tgc_new: will restore global values\n");
	m_inf->size = count; /* count of offsets */

	/* restore old global variable values */
	new_obj_flags->new_active = old_new_active;
	new_obj_flags->nesting_level--;
	new_obj_flags->no_active = old_no_active;

	dprintf("\tgc_new : before remove\n");
	/*return ptr on allocated space, begining value */
	return gc_ptr<T>((T*)((char *)res + sizeof(meta<T>)));
}