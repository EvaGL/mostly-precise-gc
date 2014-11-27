/*************************************************************************************************//**
		* File: stack.h
		* Description: This file describes memory pool, represented as mapped continued memory area
						Is not a singleton, but has some 'looks like' --- remainder of luxury
*****************************************************************************************************/
#pragma once
#include <unistd.h>

/**
* @structure --- represents a one stack element;
* @field addr --- is a pointer on the approptiate gc_ptr
*/
struct StackElement {
	void * addr;
};

class StackMap;

/**
* @class realizes an Iterator for StackElement-s in StackMap
*/
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

/**
* @class --- represents mapped continued memory pool
*/
class StackMap {
protected:
	StackMap(size_t length1,
			int free_page_parameter1,
			int add_page_parameter1
			);
public:
	static StackMap * getInstance(
		int free_page_parameter1 = -1, int add_page_parameter1 = -1,
		size_t length1 = sysconf(_SC_PAGE_SIZE)
		);

	~StackMap();

	/// add new element
	/// @param stored pointer
	void register_stack_root (void * newAddr);

	/// delete last-added element
	void delete_stack_root (void * address);

	void set_length		(size_t new_size);
	size_t get_length	();
	void set_page_size	(int new_page_size);
	Iterator begin		();
	Iterator end		();

private:
	StackMap			(StackMap const&);	// copy constructor; Not implemented because of sigleton
	StackMap& operator= (StackMap const&);	// assignment operator; Not implemented because of sigleton
	/// singleton instance
	static StackMap * instance;

	int page_size;
	/// difference between top and end_of_free_space, after witch last memory page would be unmapped
	int free_page_parameter;
	/// difference between top and end_of_free_space, after witch new memory page would be mapped
	int add_page_parameter;
	/// one page length
	size_t length;
	/// sizeof StackElement
	size_t stackElement_size;
	/// pointer to the mapped memory begin
	StackElement * map_begin;
	/// pointer to last stored element in the pool
	StackElement * top;
	/// pointer to the end of mapped memory
	StackElement * end_of_mapped_space;
};