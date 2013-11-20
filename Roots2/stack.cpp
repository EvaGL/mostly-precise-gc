/*************************************************************************************************//**
        * File: stack.cpp
        * Description: This file describes memory pool, represented as mapped continued memory area
        * Update: 20/10/13
*****************************************************************************************************/
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#define DEBUG_MODE false

struct StackElement {
	void* addr; //< stored pointer
};

class StackMap;

class Iterator {
	StackElement* se;
public:
	Iterator(StackElement* x) : se(x) {}
	Iterator(const Iterator& it) : se(it.se) {}
	Iterator& operator++() { se++; return *this; }
	Iterator operator++(int i) {
		Iterator tmp(*this);
		operator++();
		return tmp;
	}
	bool operator<=(const Iterator& it) { return (long)se <= (long)it.se; }
	bool operator>=(const Iterator& it) { return (long)se >= (long)it.se; }
	bool operator==(const Iterator& it) { return se == it.se; }
	bool operator!=(const Iterator& it) { return se != it.se; }
	void* operator*() { return se->addr; }
};

class StackMap {
protected:
	StackMap(size_t length1,
	 		int free_page_parameter1,
	 		int add_page_parameter1
			):  length(length1) {
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
public:
	static StackMap create_StackMap_instance(
			int free_page_parameter1 = -1, int add_page_parameter1 = -1,
			size_t length1 = sysconf(_SC_PAGE_SIZE)
			) {
		if (StackMap::instance) {
			return * StackMap::instance;
		}
		return StackMap(length1, free_page_parameter1, add_page_parameter1);
	}

	~StackMap() {
		StackMap::instance = NULL;
	}

	/// add new element
	/// @param stored pointer
	void register_stack_root(void* newAddr) {
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
	}

	/// delete last-added element
	void delete_stack_root() {
		if (DEBUG_MODE)
			assert(top != map_begin);
		top --;
		if (top + free_page_parameter < end_of_mapped_space)
		{
			if (munmap(end_of_mapped_space, length) == 0) {
				end_of_mapped_space -= page_size;
			} else if (DEBUG_MODE) {
				printf("StackMap: WARNING: memory page cannot be free ?!? \n");
				exit(1);
			}
		}
	}

	void set_length(size_t new_size) {
		length = new_size;
		page_size = length / stackElement_size;
	}

	size_t get_length() {
		return length;
	}

	void set_page_size(int new_page_size) {
		page_size = new_page_size;
		length = page_size * stackElement_size;
	}

	Iterator begin(){
		return Iterator(map_begin + 1);
	}
	Iterator end(){
		return Iterator(top);
	}

private:
	static StackMap* instance;
	int page_size; /// size of creating page in StackElement counts
	int free_page_parameter; /// difference between top and end_of_free_space, after witch last memory page would be unmapped
	int add_page_parameter; /// difference between top and end_of_free_space, after witch new memory page would be mapped
	size_t length; /// page length
	size_t stackElement_size; /// sizeof StackElement
	StackElement* map_begin; /// pointer to the mapped memory begin
	StackElement* top; /// pointer to last stored element
	StackElement* end_of_mapped_space; /// pointer to the end of mapped memory
};