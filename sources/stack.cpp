/*************************************************************************************************//**
		* File: stack.cpp
		* Description: includes realized primitives from stack.h
*****************************************************************************************************/
#include "stack.h"
#include <stdio.h>
#include <sys/mman.h>
#include <assert.h>

// #define DEBUG_MODE

StackMap * StackMap::instance = NULL;

StackMap::StackMap(size_t length1,
		int free_page_parameter1,
		int add_page_parameter1
		) {
#ifdef DEBUG_MODE
	printf("StackMap::StackMap(size_t length1,...\n");
	fflush(stdout);
#endif
	stackElement_size = sizeof(struct StackElement);
	page_size = length1 / stackElement_size;
	length = page_size * stackElement_size;
	if (free_page_parameter1 == -1)
		free_page_parameter = page_size * 3;
	else
		free_page_parameter = free_page_parameter1;
	if (add_page_parameter1 == -1)
		add_page_parameter = 0.3 * page_size;
	else
		add_page_parameter = add_page_parameter1;

	// allocate memory page
	map_begin = (struct StackElement*) mmap(0, length, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	assert(map_begin != MAP_FAILED);
	top = map_begin;
	end_of_mapped_space = map_begin + page_size;
#ifdef DEBUG_MODE
	printf("StackMap: allocate first memory page: %p\n", map_begin);
	fflush(stdout);
#endif
}

StackMap * StackMap::getInstance(
		int free_page_parameter1, int add_page_parameter1,
		size_t length1
		) {
#ifdef DEBUG_MODE
	printf("StackMap StackMap::create_StackMap_instance(...\n");
	fflush(stdout);
#endif
	// if (StackMap::instance) {
	// 	return * StackMap::instance;
	// }
	// return StackMap(length1, free_page_parameter1, add_page_parameter1);
	if (!StackMap::instance) {
		StackMap::instance = new StackMap(length1, free_page_parameter1, add_page_parameter1);
	}
	return StackMap::instance;
}

StackMap::~StackMap() {}

void StackMap::register_stack_root(void * newAddr) {
#ifdef DEBUG_MODE
	printf("void StackMap::register_stack_root(void* newAddr = %p) {\n", newAddr);
	fflush(stdout);
#endif
	if (top + add_page_parameter > end_of_mapped_space) {
		// mmap one more memory page
		StackElement * temp = (struct StackElement *) mmap(end_of_mapped_space, length, PROT_WRITE | PROT_READ, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		assert(temp != MAP_FAILED);
		assert(temp == end_of_mapped_space);

#ifdef DEBUG_MODE
	printf("StackMap: allocate one more memory page: ... %p -- new end ... ", (end_of_mapped_space + page_size));
	fflush(stdout);
#endif

		end_of_mapped_space = temp + page_size;
	} 

#ifdef DEBUG_MODE
	printf("top - map_begin = %li \n", (long)top - (long)map_begin);
	fflush(stdout);
#endif

	top ++;
	top->addr = newAddr;
#ifdef DEBUG_MODE
	printf("top - map_begin = %li \n", (long)top - (long)map_begin);
	fflush(stdout);
#endif
	assert(top >= map_begin);
#ifdef DEBUG_MODE
	printf("done\n");
	fflush(stdout);
#endif
}

void StackMap::delete_stack_root(void * address) {
#ifdef DEBUG_MODE
	printf("void StackMap::delete_stack_root() { %p\n", top->addr);
	fflush(stdout);
#endif
	if (top->addr != address) {
		// if it is so, then automatic objects dectructs
		// not in the reverse order with their constructors
		// so we need to find and replace object that might be deleted
		// by object that is on the top
	#ifdef DEBUG_MODE
		printf("wrong element on the top "); fflush(stdout);
	#endif
		StackElement * temp = top;
		while (temp >= map_begin) {
			if (temp->addr == address) {
				temp->addr = top->addr;
		#ifdef DEBUG_MODE
			printf("; found correct\n"); fflush(stdout);
		#endif
				break;
			}
			temp -- ;
		}
	#ifdef DEBUG_MODE
		if (temp->addr != top->addr) {
			printf(";does not correct\n"); fflush(stdout);
		}
	#endif
	}

	top --;
	assert(top >= map_begin);
	if (top + free_page_parameter < end_of_mapped_space)
	{
		if (munmap(end_of_mapped_space, length) == 0) {
			end_of_mapped_space -= page_size;
			assert(end_of_mapped_space >= map_begin);
	#ifndef DEBUG_MODE
		}
	#else
		} else {
			printf("StackMap: WARNING: memory page cannot be free ?!? \n");
			fflush(stdout);
		}
	#endif
	}
#ifdef DEBUG_MODE
	printf("top - map_begin = %li \n", (long)top - (long)map_begin);
	fflush(stdout);
#endif
}

void StackMap::set_length(size_t new_size) {
#ifdef DEBUG_MODE
	printf("void StackMap::set_length(size_t new_size) {\n");
	fflush(stdout);
#endif
	length = new_size;
	page_size = length / stackElement_size;
}

size_t StackMap::get_length() {
#ifdef DEBUG_MODE
	printf("size_t StackMap::get_length()\n");
	fflush(stdout);
#endif
	return length;
}

void StackMap::set_page_size(int new_page_size) {
#ifdef DEBUG_MODE
	printf("void StackMap::set_page_size(int new_page_size) {{\n");
	fflush(stdout);
#endif
	page_size = new_page_size;
	length = page_size * stackElement_size;
}

Iterator StackMap::begin(){
#ifdef DEBUG_MODE
	printf("Iterator StackMap::begin(){\n");
	fflush(stdout);
#endif
	return Iterator(map_begin + 1);
}

Iterator StackMap::end(){
#ifdef DEBUG_MODE
	printf("Iterator StackMap::end(){\n");
	fflush(stdout);
#endif
	return Iterator(top);
}