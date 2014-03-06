/* models that we need to keep informations about pointers */

#pragma once
#include <stdio.h>

# define MODEL_NUMBER_WIDTH  4  
# define SIZE      sizeof(size_t) - 4
# define NUM_OF_EL sizeof(size_t)
# define TAG_MODEL_1 0x1
# define TAG_MODEL_2 0x2
# define TAG_MODEL_3 0x3
# define TAG_MODEL_4 0x4

typedef unsigned long long word_t;

typedef struct {
	unsigned model     : MODEL_NUMBER_WIDTH;
	size_t   size      : SIZE;
	size_t   num_of_el : NUM_OF_EL;
} BLOCK_TAG;

typedef struct {
	size_t offset;
	int    boxed;
} POINTER_DESCR;

typedef struct {
	void * object;
	size_t current;
} PTR_ITERATOR;

void *       create_generic_object (size_t descr_length, size_t size, size_t num_of_el);
void *       create_boxed_array    (size_t size);
void *       create_unboxed_array  (size_t size);
void         set_ptr_descr         (void * object, unsigned char iter_p, POINTER_DESCR descr);
PTR_ITERATOR get_iterator          (void * object);
void *       next_ptr              (PTR_ITERATOR * iterator);
void *       get_ptr               (void * object, size_t index); 