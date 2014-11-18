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

extern StackMap stack_ptr;
extern bool new_active;
extern bool no_active;
extern size_t counter;
extern std::vector <size_t> offsets;
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
template <class T, typename ... Types>
gc_ptr<T> gc_new (Types ... types, size_t count = 1) {
#ifdef DEBUGE_MODE
	printf("gc_new start %i\n", nesting_level);
#endif
	size_t old_current_pointer_to_object = current_pointer_to_object;
	bool old_no_active = no_active;
	void *res = NULL; /* initialize object which will be store the result */
	assert(count >= 0);

	new_active = true;  /* set flag that object creates(allocates) in heap */
	/* save old offsets */
	std::vector<size_t> temp;
	temp.swap(offsets);

	const char * typeidName = typeid(T).name();
	void * clMeta = contains(classMeta, typeidName); // get pointer to class meta or NULL if it is no meta for this class
	meta<T>* m_inf = NULL;

#ifdef DEBUGE_MODE
	printf("malloc: "); fflush(stdout);
#endif

	res = my_malloc(sizeof(T) * count + sizeof(void*) + sizeof(meta<T>));  /* allocate space */
	nesting_level++;

#ifdef DEBUGE_MODE
	printf("create object %p", res); fflush(stdout);
#endif
	/* save current pointer to the allocating object */
	current_pointer_to_object = reinterpret_cast <size_t> (res + sizeof(void*) + sizeof(meta<T>));
#ifdef DEBUGE_MODE
	printf("casted "); fflush(stdout);
#endif
	transfer_to_automatic_objects(res);
#ifdef DEBUGE_MODE
	printf(" transfered ro automatic\n"); fflush(stdout);
#endif
	if (count == 1) {
	#ifdef DEBUGE_MODE
		printf("single object: "); fflush(stdout);
		new ((char *)res + sizeof(void*) + sizeof(meta<T>)) T(types ... );  /* create object in allocated space, call gc_ptr constructor, get new struct offsets */
		printf("templ_new; "); fflush(stdout);
		*((size_t*)((char *)res + sizeof(meta<T>))) =  reinterpret_cast <size_t> (new (res) meta<T>);  /* initialize meta in obj */
		printf("init meta; "); fflush(stdout);
		m_inf = reinterpret_cast <meta<T>* > (res);  /* stored pointer on meta */
		printf("cast meta; "); fflush(stdout);
	#else
		new ((char *)res + sizeof(void*) + sizeof(meta<T>)) T(types ... );  /* create object in allocated space, call gc_ptr constructor, get new struct offsets */
		*((size_t*)((char *)res + sizeof(meta<T>))) =  reinterpret_cast <size_t> (new (res) meta<T>);  /* initialize meta in obj */
		m_inf = reinterpret_cast <meta<T>* > (res);  /* stored pointer on meta */
	#endif
	#ifdef DEBUGE_MODE
			// print offsets
		printf("offsets.size() = %i\n", offsets.size());
		printf("gc_new: offsets: "); fflush(stdout);
		for (int i = 0; i < offsets.size(); i++) {
			printf("%zu ", offsets[i]);
		}
		printf("\n");
	#endif
		if (offsets.empty()){
			if (clMeta) {
				m_inf->shell = clMeta; 
			} else {
				m_inf->shell = create_generic_object(0, 0, 1); /* create new type - box, save in shell */
				addNewClassMetaInformation(typeidName, m_inf->shell);
			}
		} else {
			if (clMeta) {
				m_inf->shell = clMeta;
			} else {
				/*create new box and save pointer in shell */
				m_inf->shell = generic_box_struct (offsets, sizeof(T), count);
				addNewClassMetaInformation(typeidName, m_inf->shell);
			}
		}
	} else {
	#ifdef DEBUGE_MODE
		printf("array: "); fflush(stdout);
	#endif
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

	m_inf->size = count;  /* count of offsets */
	m_inf->ptr = (T*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>)); /*set pointer on value of object(gc_ptr) */
	m_inf->ptrptr = (void *)((char *)res + sizeof(base_meta*) + sizeof(meta<T>));

	/* restore old global variable values */
	new_active = false;
	offsets.swap(temp);
	current_pointer_to_object = old_current_pointer_to_object;
	nesting_level--;
	no_active = old_no_active;
#ifdef DEBUGE_MODE
	printf("gc_new end %i\n", nesting_level); fflush(stdout);
#endif
	/*return ptr on allocated space, begining value */

	return gc_ptr<T>((T*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>)));
}