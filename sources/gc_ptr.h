/*************************************************************************************//**
        * File: gc_ptr.h
        * Description: This file describe smart pointer class gc_ptr
		* Last modification: 16/10/13
*****************************************************************************************/
#pragma once
#include "collect.h"
#include <cstdio>
#include "stack.h"
#include <stdint.h>

#define DEBUGE_MODE

#define set_stack_flag(x)	((uintptr_t)x | (uintptr_t)1)
#define is_stack_pointer(x)	((uintptr_t)x & (uintptr_t)1)
#define get_ptr(x)			((uintptr_t)x & ~(uintptr_t)1)

extern StackMap stack_ptr;
extern std::vector <void *> offsets;
extern bool new_active; /**< global flag. False -- out gc_new, true -- in gc_new*/
extern bool no_active;

void* current_sp(void) {
	int temp;
	return &temp;
}

/**
* \class template smart pointer class gc_ptr
* \brief the class describes smart pointer
* \detailed template smart pointer class gc_ptr using to represent pointers and 
* 	override arithmetics and other operations on them.
*/
template <class T> 
class gc_ptr {
public:
	T* ptr; /**< pointer on specified type*/

	/**	\fn construct gc_ptr()
		\brief setting ptr on null
	*/
	gc_ptr() {
		ptr = 0;
		if (!no_active) {
			if (!new_active) {
				inc(this);
				ptr = (T *) set_stack_flag(ptr);
			} else {
				if (!((size_t)this > (size_t)&stack_ptr && (size_t)this <= (size_t)current_sp )) {
					offsets.push_back(this);
				}
			}
		}
	}

	/**	\fn construct gc_ptr(int* p)
		\brief setting  pointer pointers on p pointer type of T 			
	*/
	gc_ptr(T* p) {
		ptr = p;
		if (!no_active) {
			if (!new_active) {
				inc(this);
				ptr = (T *) set_stack_flag(ptr);
			} else {
				if (!((size_t)this > (size_t)&stack_ptr && (size_t)this <= (size_t)current_sp )) {
					offsets.push_back(this);
				}
			}
		}
	}

	/**	\fn construct gc_ptr(const gc_ptr<int> &p)
		\brief setting pointer on given adress type of T 			
	*/
	gc_ptr(const gc_ptr <T> &p) {
		ptr = (T*) get_ptr(p.ptr);
		if (!no_active) {
			if (!new_active) {
				inc(this);
				ptr = (T *) set_stack_flag(ptr);
			} else {
				if (!((size_t)this > (size_t)&stack_ptr && (size_t)this <= (size_t)current_sp )) {
					offsets.push_back(this);
				}
			}
		}
	}
	
	/**	\fn destructor gc_ptr()
		\brief delete current gc_ptr from special ptr_list  			
	*/
	~gc_ptr() {
		if (is_stack_pointer(ptr)) {
			dec();
		}
	}


	/* reloaded operators for gc_ptrs objects*/
	T& operator*() const 					{	return * (T*) get_ptr(ptr); }
	T* operator->() const 					{	return (T*) get_ptr(ptr); }
	operator T* () const 					{	return (T*) get_ptr(ptr); }
	T* get() const 							{	return (T*) get_ptr(ptr); }
	T& operator[](size_t index) const 		{	return ((T*) get_ptr(ptr))[index]; }
	T& operator[](size_t index) 			{	return ((T*) get_ptr(ptr))[index]; }
	bool operator == (const gc_ptr <T> &a) 	{	return (T*) get_ptr(a.ptr) == (T*) get_ptr(ptr); }
	bool operator != (const gc_ptr <T> &a) 	{	return (T*) get_ptr(a.ptr) != (T*) get_ptr(ptr); }
	bool operator != (const T* a) 			{	return a != (T*) get_ptr(ptr); }
	bool operator == (const T* a) 			{	return a == (T*) get_ptr(ptr); }
	bool operator == (const int a) 			{	return a == reinterpret_cast<size_t> ((T*) get_ptr(ptr)); }
	bool operator != (const int a) 			{	return a != reinterpret_cast<size_t> ((T*) get_ptr(ptr)); }
	bool operator == (const long int a)		{	return a == reinterpret_cast<size_t> ((T*) get_ptr(ptr)); }
	bool operator != (const long int a)		{	return a != reinterpret_cast<size_t> ((T*) get_ptr(ptr)); }
	operator bool() const 					{	return (T*) get_ptr(ptr) != NULL; }

	gc_ptr& operator = (const gc_ptr <T> &a) {
		if (is_stack_pointer(ptr)) {
			ptr = (T*) get_ptr(a.ptr);
			ptr = (T*) set_stack_flag(ptr);
		} else {
			ptr = (T*) get_ptr(a.ptr);
		}
		return *this;
	}

	gc_ptr& operator = (T *a) {
		if (is_stack_pointer(ptr)) {
			ptr = a;
			ptr = (T*) set_stack_flag(ptr);
		} else {
			ptr = a;
		}
		return *this;
	}
};
