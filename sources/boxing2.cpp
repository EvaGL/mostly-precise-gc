/*************************************************************************************//**
	* File: boxing.cpp
	* Description: This file consists code with realiasation of functions described in "boxing.h"
*****************************************************************************************/

#include "boxing2.h"
#include <iterator>
#include "taginfo.h"
#include <cstdio>

/** 
* @brief generation box(special meta-info object) for one-word object
* @detailed return the pointer on object which have created with create_generic_object function
*/
void * generic_box_simple () { 
	/*
	 * call generic function for creating object in one-word
	 * first arg - num of pointers descr, sec arg - size(depend from num of pointers descr), thd arg - num words
	 */
	return create_generic_object(0, 0, 1); 
}

/** 
* @brief generation box for arrays with unboxed elements
* @detailed create the pointer on object with create_unboxed_array function
* @return the pointer on object
* @param len - num of elements
*/
void * generic_box_unboxed_array(size_t len) {
	/*
	 * call the function for creating object with unboxed elements
	 * len - num of unboxed elements in array
	 */
	return create_unboxed_array(len);
}

/** 
* @brief generation box for arrays with unboxed elements
* @detailed create the pointer on object with create_unboxed_array function
* @param len - num of elements
* @return the pointer on object
*/
void * generic_box_boxed_array (size_t len, void * clMeta, size_t typeSize) { 
	/*
	 * call the function for creating object with boxed elements
	 * first arg - num of boxed elements in array
	 */	
	return create_boxed_array(len, clMeta, typeSize);
}

/** 
* @brief generation box for struct
* @detailed  create the pointer on object with create_generic_object function
* @param offsets_ptr - list of pointer offsets in struct, size - full size of real struct, num_el - num of this type elements!!!! ?num of pointers 
* @return the pointer on object
*/
void * generic_box_struct (std::vector <size_t> offsets_ptr, size_t size, size_t num_el) {
	void* object; /**< a stored pointer */
	try {
		/*
		 * call function creating box object for struct > 
		 * first arg - num of offsets, sec arg - size of real struct, thd arg - num of pointers in the struct>
		 */
		object = create_generic_object(offsets_ptr.size(), size, num_el);
		#ifdef DEBUGE_MODE
			printf("%zu\n", offsets_ptr.size());
		#endif
		std::vector <size_t>::iterator it_offset = offsets_ptr.begin(); /**< create iterator for offsets_ptr*/
		POINTER_DESCR descr; /**< temprorary element for saving offset*/
		for ( size_t iter_p = 1; iter_p < offsets_ptr.size() + 1; iter_p++, it_offset++) { /* save all pointers in object */
			descr = {*it_offset, 0}; /* save pointers in descriptor */
			/*
			 * call function write descriptor in object 
			 * first arg - object reference, sec arg - index place in object, thd arg - descriptor
			 */
			set_ptr_descr(object, iter_p, descr); 
		}
	}
	catch (...) {
		printf("Error! Couldn't create generic box struct!");
	}
	return object;	
}
