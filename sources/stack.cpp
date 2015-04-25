/*************************************************************************************************//**
		* File: stack.cpp
		* Description: includes realized primitives from stack.h
*****************************************************************************************************/
#include "stack.h"
#include <stdio.h>
#include <sys/mman.h>
#include <assert.h>

// StackMap * StackMap::instance = NULL;
// std::mutex StackMap::mutex;

StackMap::StackMap(size_t length1,
		int free_page_parameter1,
		int add_page_parameter1
		) {
	dprintf("StackMap::StackMap(size_t length1,...\n");
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
	dprintf("StackMap: allocate first memory page: %p\n", map_begin);
}

StackMap * StackMap::getInstance(
		int free_page_parameter1, int add_page_parameter1,
		size_t length1
		) {
	static thread_local StackMap instance(length1, free_page_parameter1, add_page_parameter1);
	return &instance;
	// std::lock_guard<std::mutex> lock (mutex);
	// if (!StackMap::instance) {
	// 	StackMap::instance = new StackMap(length1, free_page_parameter1, add_page_parameter1);
	// }
	// return StackMap::instance;
}

StackMap::~StackMap() {}

void StackMap::register_stack_root(void * newAddr) {
	dprintf("void StackMap::register_stack_root(void* newAddr = %p) {\n", newAddr);
	if (top + add_page_parameter > end_of_mapped_space) {
		// mmap one more memory page
		StackElement * temp = (struct StackElement *) mmap(end_of_mapped_space, length, PROT_WRITE | PROT_READ, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		assert(temp != MAP_FAILED);
		assert(temp == end_of_mapped_space);

		dprintf("StackMap: allocate one more memory page: ... %p -- new end ... ", (end_of_mapped_space + page_size));
		end_of_mapped_space = temp + page_size;
	} 

	dprintf("top - map_begin = %li \n", (long)top - (long)map_begin);
	top ++;
	top->addr = newAddr;
	dprintf("top - map_begin = %li \n", (long)top - (long)map_begin);

	assert(top >= map_begin);
	dprintf("register_stack_root::done\n");
}

void StackMap::delete_stack_root(void * address) {
	dprintf("void StackMap::delete_stack_root() { %p\n", top->addr);
	if (top->addr != address) {
		// if it is so, then automatic objects dectructs
		// not in the reverse order with their constructors
		// so we need to find and replace object that might be deleted
		// by object that is on the top
		dprintf("wrong element on the top ");
		StackElement * temp = top;
		while (temp >= map_begin) {
			if (temp->addr == address) {
				temp->addr = top->addr;
				dprintf("StackMap::delete_stack_root:: found correct\n");
				break;
			}
			temp -- ;
		}
	#ifdef DEBUGE_MODE
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
		} else {
			dprintf("StackMap: WARNING: memory page cannot be free ?!? \n");
		}
	}
	dprintf("top - map_begin = %li \n", (long)top - (long)map_begin);
}

void StackMap::set_length(size_t new_size) {
	dprintf("void StackMap::set_length(size_t new_size) {\n");
	length = new_size;
	page_size = length / stackElement_size;
}

size_t StackMap::get_length() {
	dprintf("size_t StackMap::get_length()\n");
	return length;
}

void StackMap::set_page_size(int new_page_size) {
	dprintf("void StackMap::set_page_size(int new_page_size) {{\n");
	page_size = new_page_size;
	length = page_size * stackElement_size;
}

Iterator StackMap::begin(){
	dprintf("Iterator StackMap::begin(){\n");
	return Iterator(map_begin + 1);
}

Iterator StackMap::end(){
	dprintf("Iterator StackMap::end(){\n");
	return Iterator(top);
}