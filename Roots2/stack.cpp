#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

struct StackElement {
	void* addr;
};

class StackMap {
public:
	StackMap() {
		stackElement_size = sizeof(struct StackElement);
		length = sysconf(_SC_PAGE_SIZE);
		page_size = length / stackElement_size;

		map_begin = (struct StackElement*) mmap(sbrk(0), length, PROT_WRITE | PROT_READ, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		top = map_begin;
		end_of_mapped_space = map_begin + page_size;
	}

	bool inc(void* newAddr) {
		if (top + 20 == end_of_mapped_space) {
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
	int count = 0;
	int page_size;// сколько элементов  в одном mmap выделится
	StackElement* map_begin;
	StackElement* top;
	size_t length;
	size_t stackElement_size;
	StackElement* end_of_mapped_space;
};