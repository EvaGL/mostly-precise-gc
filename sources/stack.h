/*************************************************************************************************//**
        * File: stack.h
        * Description: This file describes memory pool, represented as mapped continued memory area
*****************************************************************************************************/
#pragma once
#include <unistd.h>

struct StackElement {
	void * addr; //< stored pointer
};

class StackMap;

class Iterator {
	StackElement * se;
public:
	Iterator (StackElement * x) : se(x)	{}
	Iterator (const Iterator& it) : se(it.se) {}
	Iterator& operator++ () { se++; return *this; }
	Iterator operator++ (int i) {
		Iterator tmp(*this);
		operator++();
		return tmp;
	}
	bool operator<= (const Iterator& it)	{ return (long)se <= (long)it.se; }
	bool operator>= (const Iterator& it)	{ return (long)se >= (long)it.se; }
	bool operator== (const Iterator& it)	{ return se == it.se; }
	bool operator!= (const Iterator& it)	{ return se != it.se; }
	void * operator* ()						{ return se->addr; }
};

class StackMap {
protected:
	StackMap(size_t length1,
	 		int free_page_parameter1,
	 		int add_page_parameter1
			);
public:
	static StackMap create_StackMap_instance(
		int free_page_parameter1 = -1, int add_page_parameter1 = -1,
		size_t length1 = sysconf(_SC_PAGE_SIZE)
		);

	~StackMap() {
		StackMap::instance = NULL;
	}

	/// add new element
	/// @param stored pointer
	void register_stack_root (void * newAddr);

	/// delete last-added element
	void delete_stack_root ();

	void set_length		(size_t new_size);
	size_t get_length	();
	void set_page_size	(int new_page_size);
	Iterator begin		();
	Iterator end		();

private:
	static StackMap* instance;
	int page_size; /// size of creating page in StackElement counts
	int free_page_parameter; /// difference between top and end_of_free_space, after witch last memory page would be unmapped
	int add_page_parameter; /// difference between top and end_of_free_space, after witch new memory page would be mapped
	size_t length; /// page length
	size_t stackElement_size; /// sizeof StackElement
	StackElement * map_begin; /// pointer to the mapped memory begin
	StackElement * top; /// pointer to last stored element
	StackElement * end_of_mapped_space; /// pointer to the end of mapped memory
};