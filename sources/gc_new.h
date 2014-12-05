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
#include <msmalloc.h>
#include "gc_ptr.h"

extern std::vector <size_t> offsets;
extern bool new_active;
extern bool no_active;
extern size_t counter;
extern MetaInformation * classMeta;
extern int nesting_level;
extern size_t current_pointer_to_object;

// #define DEBUGE_MODE

/**
* @class base_meta
* @brief object meta base class
* @detailed realizes base object meta class (like interface for eatch object meta)
*/
class base_meta {
public:
	void *shell;	/**< pointer on the box(meta info struct for storing offsets) of object */
	void * ptrptr;	/**< pointer to the real object begin */
	size_t size;	/**< size of object */
	virtual void del_ptr () = 0;	/**< delete meta-ptr */
	virtual void* get_begin () = 0;	/**< get begin of object (pointer on meta)*/
};

/**
* @class meta  
* @brief template class; realizes specific meta for eatch object;
* @detailed it creates in gc_new and it it stored directly with (right before) the
	allocated object.
*/
template <class T>
class meta : public base_meta {
public:
	T* ptr;	/**< "typed" pointer to the real object begin */

	/// virtual del_ptr function from base_meta realization
	void del_ptr (void) {
	#ifdef DEBUGE_MODE
		printf("in del_ptr\n"); fflush(stdout);
	#endif
		if (size == 1) {
			((T*)ptr)->~T();
		} else {
			for (size_t i = 0; i < size; i++)
				((T*)ptr)[i].~T();
		}
	}

	/// virtual get_begin function from base_meta realization
	void* get_begin (void) {
	#ifdef DEBUGE_MODE
		printf("in get_begin\n"); fflush(stdout);
	#endif
		return reinterpret_cast <void*> (this);
	}
};

/**
* @function hasOffsets
* @brief checks has class T some gc_ptr in or not
* @detailed
* @return bool: true --- calss T has some offsets, false --- otherwise
*/
template <class T>
bool hasOffsets (void) {
#ifdef DEBUGE_MODE
	printf("in hasOffsets\n"); fflush(stdout);
#endif
	nesting_level++;
	/* save global variable values*/
	std::vector<size_t> temp;
	temp.swap(offsets);
	size_t old_current_pointer_to_object = current_pointer_to_object;
	const char * typeidName = typeid(T).name();
	void * clMeta = contains(classMeta, typeidName);
	/* allocate space */
	void * res = my_malloc(sizeof(T) + sizeof(void*) + sizeof(meta<T>));
	current_pointer_to_object = reinterpret_cast <size_t> (res + sizeof(void*) + sizeof(meta<T>));
	transfer_to_automatic_objects(res);
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
			addNewClassMetaInformation(typeidName, m_inf->shell);
		}
	} else {
		if (clMeta) {
			m_inf->shell = clMeta;
		} else {
			m_inf->shell = generic_box_struct (offsets, sizeof(T), 1);
			addNewClassMetaInformation(typeidName, m_inf->shell);
		}
	}

	bool result = (!offsets.empty());
	/* restore old global variable values */
	temp.swap(offsets);
	current_pointer_to_object = old_current_pointer_to_object;
	nesting_level--;
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
#ifndef DEBUGE_MODE
template <class T, typename ... Types>
gc_ptr<T> gc_new (Types ... types, size_t count = 1) {
	assert(count >= 0);
	const char * typeidName = typeid(T).name();
	// get pointer to class meta or NULL if it is no meta for this class
	void * clMeta = contains(classMeta, typeidName);
	/* set global active flags */
	bool old_new_active = new_active;
	new_active = true;  /* set flag that object creates(allocates) in heap */
	bool old_no_active = no_active;
	/* clMeta != NULL => no_active == true; false --- otherwise */
	no_active = clMeta;
	/* metainformation that will be stored directly with object */
	meta<T>* m_inf = NULL;
	/* initialize object which will be store the result and allocate space */
	void * res = my_malloc(sizeof(T) * count + sizeof(void*) + sizeof(meta<T>));
	transfer_to_automatic_objects(res);
	nesting_level++;
	if (clMeta != NULL) {
		if (count == 1) {
			new ((char *)res + sizeof(void*) + sizeof(meta<T>)) T(types ... );  /* create object in allocated space, call gc_ptr constructor, get new struct offsets */
			*((size_t*)((char *)res + sizeof(meta<T>))) =  reinterpret_cast <size_t> (new (res) meta<T>);  /* initialize meta in obj */
			m_inf = reinterpret_cast <meta<T>* > (res); /* stored pointer on meta */
			m_inf->shell = clMeta;
		} else {
			no_active = true; // offsets wont counting yet
			new ((char *)res + sizeof(void *) + sizeof(meta<T>)) T[count];
			*((size_t*)((char *)res + sizeof(meta<T>))) = reinterpret_cast <size_t> (new (res) meta<T>);
			m_inf = reinterpret_cast <meta<T>* > (res);

			no_active = true; // offsets will counting yet
			new_active = true; // restore, because string new ... T[coint]; changed it to false
			BLOCK_TAG* tag = (BLOCK_TAG *) clMeta; /* get meta tag */
			size_t offsets_count = 0;
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
				m_inf->shell = create_boxed_array(count, clMeta, sizeof(T));
			} else {
				m_inf->shell = create_unboxed_array(count);
			}
		}

	} else {
	/* in this case we might to create clMeta (class meta information) and save in in ClassMetaList */
		/* save old offsets */
		std::vector<size_t> temp;
		temp.swap(offsets);
		/* save current pointer to the allocating object; it uses in gc_ptr constructor to evaluate offsets size */
		size_t old_current_pointer_to_object = current_pointer_to_object;
		current_pointer_to_object = reinterpret_cast <size_t> (res + sizeof(void*) + sizeof(meta<T>));

		if (count == 1) {
			new ((char *)res + sizeof(void*) + sizeof(meta<T>)) T(types ... );  /* create object in allocated space, call gc_ptr constructor, get new struct offsets */
			*((size_t*)((char *)res + sizeof(meta<T>))) =  reinterpret_cast <size_t> (new (res) meta<T>);  /* initialize meta in obj */
			m_inf = reinterpret_cast <meta<T>* > (res);  /* stored pointer on meta */

			if (offsets.empty()) {
				m_inf->shell = create_generic_object(0, 0, 1); /* create new type - box, save in shell */
			} else {
				/*create new box and save pointer in shell */
				m_inf->shell = generic_box_struct (offsets, sizeof(T), count);
			}
			addNewClassMetaInformation(typeidName, m_inf->shell);
		} else {
			no_active = true; // offsets wont counting yet
			new ((char *)res + sizeof(void *) + sizeof(meta<T>)) T[count];
			*((size_t*)((char *)res + sizeof(meta<T>))) = reinterpret_cast <size_t> (new (res) meta<T>);
			m_inf = reinterpret_cast <meta<T>* > (res);

			no_active = false; // offsets will counting yet
			new_active = true; // restore, because string new ... T[coint]; changed it to false
			// force counting class metainfo, because it can be that we have not meta for class T
			if (hasOffsets<T>()) {
				clMeta = contains(classMeta, typeidName);
				assert(clMeta != NULL);
				m_inf->shell = create_boxed_array(count, clMeta, sizeof(T));
			} else {
				clMeta = contains(classMeta, typeidName);
				m_inf->shell = create_unboxed_array(count);
			}
			assert(clMeta != NULL);
		}

		/* restore old global variable values */
		offsets.swap(temp);
		current_pointer_to_object = old_current_pointer_to_object;
	}
	m_inf->size = count; /* count of offsets */
	m_inf->ptr = (T*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>)); /*set pointer on value of object(gc_ptr) */
	m_inf->ptrptr = (void *)((char *)res + sizeof(base_meta*) + sizeof(meta<T>));

	/* restore old global variable values */
	new_active = old_new_active;
	nesting_level--;
	no_active = old_no_active;

	/*return ptr on allocated space, begining value */
	return gc_ptr<T>((T*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>)));
}
#else
template <class T, typename ... Types>
gc_ptr<T> gc_new (Types ... types, size_t count = 1) {
	assert(count >= 0);
	printf("in gc_new: \n"); fflush(stdout);
	const char * typeidName = typeid(T).name();
	// get pointer to class meta or NULL if it is no meta for this class
	void * clMeta = contains(classMeta, typeidName);

	printf("\tclMeta=%p\n", clMeta); fflush(stdout);

	/* set global active flags */
	bool old_new_active = new_active;
	new_active = true;  /* set flag that object creates(allocates) in heap */
	bool old_no_active = no_active;
	/* clMeta != NULL => no_active == true; false --- otherwise */
	no_active = clMeta;
	/* metainformation that will be stored directly with object */
	meta<T>* m_inf = NULL;
	/* initialize object which will be store the result and allocate space */
	void * res = my_malloc(sizeof(T) * count + sizeof(void*) + sizeof(meta<T>));
	transfer_to_automatic_objects(res);
	nesting_level++;
	if (clMeta != NULL) {
		printf("\tclMeta != NULL\n"); fflush(stdout);

		if (count == 1) {
			printf("\tcount == 1\n"); fflush(stdout);

			new ((char *)res + sizeof(void*) + sizeof(meta<T>)) T(types ... );  /* create object in allocated space, call gc_ptr constructor, get new struct offsets */
			*((size_t*)((char *)res + sizeof(meta<T>))) =  reinterpret_cast <size_t> (new (res) meta<T>);  /* initialize meta in obj */
			m_inf = reinterpret_cast <meta<T>* > (res); /* stored pointer on meta */
			m_inf->shell = clMeta;
		} else {
			printf("\tcount != 1\n"); fflush(stdout);

			no_active = true; // offsets wont counting yet
			new ((char *)res + sizeof(void *) + sizeof(meta<T>)) T[count];
			*((size_t*)((char *)res + sizeof(meta<T>))) = reinterpret_cast <size_t> (new (res) meta<T>);
			m_inf = reinterpret_cast <meta<T>* > (res);

			no_active = true; // offsets will counting yet
			new_active = true; // restore, because string new ... T[coint]; changed it to false
			BLOCK_TAG* tag = (BLOCK_TAG *) clMeta; /* get meta tag */
			size_t offsets_count = 0;

			printf("\ttag Model: %i\n", tag->model); fflush(stdout);

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
				printf("\toffset count != 0\n"); fflush(stdout);
				m_inf->shell = create_boxed_array(count, clMeta, sizeof(T));
			} else {
				printf("\toffset count == 0\n"); fflush(stdout);
				m_inf->shell = create_unboxed_array(count);
			}
		}
	} else {
	/* in this case we might to create clMeta (class meta information) and save in in ClassMetaList */
		printf("\tclMeta == NULL\n"); fflush(stdout);
		/* save old offsets */
		std::vector<size_t> temp;
		temp.swap(offsets);
		/* save current pointer to the allocating object; it uses in gc_ptr constructor to evaluate offsets size */
		size_t old_current_pointer_to_object = current_pointer_to_object;
		current_pointer_to_object = reinterpret_cast <size_t> (res + sizeof(void*) + sizeof(meta<T>));

		if (count == 1) {
			printf("\tcount == 1\n"); fflush(stdout);
			new ((char *)res + sizeof(void*) + sizeof(meta<T>)) T(types ... );  /* create object in allocated space, call gc_ptr constructor, get new struct offsets */
			*((size_t*)((char *)res + sizeof(meta<T>))) =  reinterpret_cast <size_t> (new (res) meta<T>);  /* initialize meta in obj */
			m_inf = reinterpret_cast <meta<T>* > (res);  /* stored pointer on meta */

			if (offsets.empty()) {
				printf("\toffsets is empty\n"); fflush(stdout);
				m_inf->shell = create_generic_object(0, 0, 1); /* create new type - box, save in shell */
			} else {
				printf("\toffsets is not empty: count == %i\n", offsets.size()); fflush(stdout);
				/*create new box and save pointer in shell */
				m_inf->shell = generic_box_struct (offsets, sizeof(T), count);
			}
			addNewClassMetaInformation(typeidName, m_inf->shell);
		} else {
			printf("\tcount != 1\n"); fflush(stdout);
			no_active = true; // offsets wont counting yet
			new ((char *)res + sizeof(void *) + sizeof(meta<T>)) T[count];
			*((size_t*)((char *)res + sizeof(meta<T>))) = reinterpret_cast <size_t> (new (res) meta<T>);
			m_inf = reinterpret_cast <meta<T>* > (res);

			no_active = false; // offsets will counting yet
			new_active = true; // restore, because string new ... T[coint]; changed it to false
			// force counting class metainfo, because it can be that we have not meta for class T
			printf("\tcall hasOffsets\n"); fflush(stdout);
			if (hasOffsets<T>()) {
				clMeta = contains(classMeta, typeidName);
				assert(clMeta != NULL);
				m_inf->shell = create_boxed_array(count, clMeta, sizeof(T));
			} else {
				clMeta = contains(classMeta, typeidName);
				m_inf->shell = create_unboxed_array(count);
			}
			assert(clMeta != NULL);
		}

		/* restore old global variable values */
		offsets.swap(temp);
		current_pointer_to_object = old_current_pointer_to_object;
	}
	printf("\tgc_new: will restore global values\n"); fflush(stdout);
	m_inf->size = count; /* count of offsets */
	m_inf->ptr = (T*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>)); /*set pointer on value of object(gc_ptr) */
	m_inf->ptrptr = (void *)((char *)res + sizeof(base_meta*) + sizeof(meta<T>));

	/* restore old global variable values */
	new_active = old_new_active;
	nesting_level--;
	no_active = old_no_active;

	printf("\tgc_new : before remove\n"); fflush(stdout);
	/*return ptr on allocated space, begining value */
	return gc_ptr<T>((T*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>)));
}
#endif