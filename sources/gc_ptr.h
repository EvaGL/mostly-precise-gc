/*************************************************************************************//**
        * File: gc_ptr.h
        * Description: This file describe smart pointer class gc_ptr
		* Last modification: 16/10/13
*****************************************************************************************/
#pragma once
#include "collect.h"
#include <cstdio>
#include "PointerList.h"

extern PointerList * offsets;
extern bool new_active; /**< global flag. False -- out gc_new, true -- in gc_new*/
extern bool no_active;
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
	bool stack_ptr; /**< True, if this pointer point on stack, False - otherwise */

	/**	\fn construct gc_ptr()
		\brief setting ptr on null
	*/
	gc_ptr() {
		if (!no_active) {
			if (!new_active) {
				// add current addr in ptr_list (list of pointers on stack)
				#ifdef DEBUGE_MODE
					printf("stack ptr\n");
				#endif
				inc(this); stack_ptr = true;
			} else {
				// add our ptr in offsets list
				stack_ptr = 0;
				offsets = pushBackToPointerList(offsets, this);
			}
		}
		ptr = 0;
	}

	/**	\fn construct gc_ptr(int* p)
		\brief setting  pointer pointers on p pointer type of T 			
	*/
	gc_ptr(T* p) {
		if (!no_active) {
			if (!new_active) {
				#ifdef DEBUGE_MODE
					printf("stack ptr\n");
				#endif
				inc(this), stack_ptr = true;
			} else {
				stack_ptr = 0;
				offsets = pushBackToPointerList(offsets, this);
			}
		}
		ptr = p;
	}

	/**	\fn construct gc_ptr(const gc_ptr<int> &p)
		\brief setting pointer on given adress type of T 			
	*/
	gc_ptr(const gc_ptr <T> &p) {
		if (!no_active) {
			if (!new_active) {
				#ifdef DEBUGE_MODE
					printf("stack ptr\n");
				#endif
				inc(this), stack_ptr = true;
			} else {
				stack_ptr = 0;
				offsets = pushBackToPointerList(offsets, this);
			}
		}
		ptr = p.ptr;
	}
	/**	\fn destructor gc_ptr()
		\brief delete current gc_ptr from special ptr_list  			
	*/
	~gc_ptr() {
		if (stack_ptr) {
			#ifdef DEBUGE_MODE
				printf("stack ptr dectr\n");
			#endif
			dec();
		}
	}

	/* reloaded operators for gc_ptrs objects*/
	T& operator*() const { return *ptr; }
	T* operator->() const {	return ptr; }
	T* get() const { return ptr; }
	T& operator[](size_t index) const { return ptr[index]; }
	T& operator[](size_t index) { return ptr[index]; }
	bool operator == (const gc_ptr <T> &a) { return (a.ptr == ptr); }
	bool operator != (const gc_ptr <T> &a) { return (a.ptr != ptr); }
	bool operator != (const T* a) { return (a != ptr); }
	bool operator == (const T* a) { return (a == ptr); }
	bool operator == (const int a) { return (a == reinterpret_cast<size_t> (ptr)); }
	bool operator != (const int a) { return (a != reinterpret_cast<size_t> (ptr)); }
	bool operator == (const long int a) { return (a == reinterpret_cast<size_t> (ptr)); }
	bool operator != (const long int a) { return (a != reinterpret_cast<size_t> (ptr)); }
	operator bool() const {	return (ptr != NULL); }

	gc_ptr& operator = (const gc_ptr <T> &a) {
		ptr = a.ptr;
		return *this;
	}

	gc_ptr& operator = (T *a) {
		ptr = a;
		return *this;
	}
};
