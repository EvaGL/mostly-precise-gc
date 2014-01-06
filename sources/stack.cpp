#include "stack.h"
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define DEBUG_MODE false

StackMap * StackMap::instance = NULL;

StackMap::StackMap(size_t length1,
 		int free_page_parameter1,
 		int add_page_parameter1
		):  length(length1) {
	if (DEBUG_MODE) {
		printf("StackMap::StackMap(size_t length1,...\n");
	}
	StackMap::instance = this;
	stackElement_size = sizeof(struct StackElement);
	page_size = length / stackElement_size;
	if (free_page_parameter1 == -1)
		free_page_parameter = page_size * 1.5;
	else
		free_page_parameter = free_page_parameter1;
	if (add_page_parameter1 == -1)
		add_page_parameter = 0.3 * page_size;
	else
		add_page_parameter = add_page_parameter1;

	// allocate memory page
	map_begin = (struct StackElement*) mmap(sbrk(0), length, PROT_WRITE | PROT_READ, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	top = map_begin;
	end_of_mapped_space = map_begin + page_size;
	if (DEBUG_MODE) {
		printf("StackMap: allocate first memory page: ");
		assert(map_begin != MAP_FAILED);
		printf("Done!\n");
	}
}

StackMap StackMap::create_StackMap_instance(
		int free_page_parameter1, int add_page_parameter1,
		size_t length1
		) {
	if (DEBUG_MODE) {
		printf("StackMap StackMap::create_StackMap_instance(");
		printf("Done!\n");
	}
	if (StackMap::instance) {
		return * StackMap::instance;
	}
	return StackMap(length1, free_page_parameter1, add_page_parameter1);
}

void StackMap::register_stack_root(void* newAddr) {
	if (DEBUG_MODE) {
		printf("void StackMap::register_stack_root(void* newAddr) {\n");
	}
	if (top + add_page_parameter == end_of_mapped_space) {
		// mmap one more memory page
		end_of_mapped_space = (struct StackElement *) mmap(end_of_mapped_space, length, PROT_WRITE | PROT_READ, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (DEBUG_MODE) {
			printf("StackMap: allocate one more memory page:  ");
			assert(end_of_mapped_space != MAP_FAILED);
			printf("Done!\n");
		}
		end_of_mapped_space += page_size;
	} 
	top ++;
	top->addr = newAddr;
	if (DEBUG_MODE) {
		printf("done\n");
		fflush(stdout);
	}
}

void StackMap::delete_stack_root() {
	if (DEBUG_MODE) {
		printf("void StackMap::delete_stack_root() {\n");
	}
	top --;
	if (top + free_page_parameter < end_of_mapped_space)
	{
		if (munmap(end_of_mapped_space, length) == 0) {
			end_of_mapped_space -= page_size;
			if (DEBUG_MODE) {
				assert(end_of_mapped_space > map_begin);
			}
		} else if (DEBUG_MODE) {
			printf("StackMap: WARNING: memory page cannot be free ?!? \n");
			exit(1);
		}
	}
	if (DEBUG_MODE) {
		printf("top - map_begin = %li \n", (long)top - (long)map_begin);
		fflush(stdout);
	}
}

void StackMap::set_length(size_t new_size) {
	if (DEBUG_MODE) {
		printf("void StackMap::set_length(size_t new_size) {\n");
	}
	length = new_size;
	page_size = length / stackElement_size;
}

size_t StackMap::get_length() {
	if (DEBUG_MODE) {
		printf("size_t StackMap::get_length()\n");
	}
	return length;
}

void StackMap::set_page_size(int new_page_size) {
	if (DEBUG_MODE) {
		printf("void StackMap::set_page_size(int new_page_size) {{\n");
	}
	page_size = new_page_size;
	length = page_size * stackElement_size;
}

Iterator StackMap::begin(){
	if (DEBUG_MODE) {
		printf("Iterator StackMap::begin(){\n");
	}
	return Iterator(map_begin + 1);
}

Iterator StackMap::end(){
	if (DEBUG_MODE) {
		printf("Iterator StackMap::end(){\n");
	}
	return Iterator(top);
}