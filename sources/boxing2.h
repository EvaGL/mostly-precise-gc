/************************************************************************************//**
    * File: boxing2.h
    * Description: Describes functions of creation boxes for different objects types
*****************************************************************************************/

#pragma once
#include <stdlib.h>
#include <limits.h>
#include <vector>

/* \def word_t
	\brief word_t besides unsigned long long 
*/
typedef unsigned long long word_t;

/* \fn create box for different kinds of objects
*/
extern void * generic_box_simple		(void); /*< create box for 1-word obj*/
extern void * generic_box_unboxed_array	(size_t len); /*< create box for array with num of chunks = len*/
extern void * generic_box_boxed_array	(size_t len, void * clMeta, size_t typeSize); /*< create box for array of pointers on boxed objecct num of chunks = len*/
/* \fn create box for struct 
	\params list of pointers in struct, size, num el
*/
extern void * generic_box_struct		(std::vector <size_t> offsets_ptr, size_t size, size_t num_el); 