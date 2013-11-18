/*************************************************************************************************//**
        * File: stack.cpp
        * Description: This file describes memory pool, represented as mapped continued memory area
        * Update: 20/10/13
*****************************************************************************************************/
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

struct StackElement {
	void* addr; //< stored pointer
};

class StackMap {
public:
	StackMap() {
		stackElement_size = sizeof(struct StackElement);
		length = sysconf(_SC_PAGE_SIZE);
		page_size = length / stackElement_size;

		// allocate memory page
		if ((map_begin = (struct StackElement*) mmap(sbrk(0), length, PROT_WRITE | PROT_READ, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
			perror("\n ERROR: at StackMap::StackMap() --- allocation space failed \n");
			map_begin = top = end_of_mapped_space = NULL;
			return;
		}
		top = map_begin;
		end_of_mapped_space = map_begin + page_size;
	}

	/// add new element
	/// @param stored pointer
	bool inc(void* newAddr) {
		if (top + 20 == end_of_mapped_space) {
			// mmap one more memory page
			end_of_mapped_space = (struct StackElement *) mmap(end_of_mapped_space, length, PROT_WRITE | PROT_READ, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			if (end_of_mapped_space == MAP_FAILED) {
				printf("\nStackMap::inc() --- cannot allocate new page\n");
				return  false;
			} else {
				end_of_mapped_space += page_size;
			}
		} 
		top ++;
		top->addr = newAddr;
		count++;
		return true;
	}

	/// delete last-added element
	bool dec() {
		if (top == map_begin) {
			return false;
		}
		top --;
		count --;
		if (top + 600 < end_of_mapped_space)
		{
			if (munmap(end_of_mapped_space, length) == 0) {
				end_of_mapped_space -= page_size;
			}
		}
		return true;
	}

	void* get_begin() {
		return map_begin;
	}

	int get_count() {
		return count;
	}

private:
	int count = 0; /// current pointers count
	int page_size; /// size of creating page in StackElement counts
	StackElement* map_begin; /// pointer to the mapped memory begin
	StackElement* top; /// pointer to last stored element
	size_t length; /// page length
	size_t stackElement_size; /// sizeof StackElement
	StackElement* end_of_mapped_space; /// pointer to the end of mapped memory
};