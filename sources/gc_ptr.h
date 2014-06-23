/*************************************************************************************//**
        * File: gc_ptr.h
        * Description: This file describe smart pointer class gc_ptr
*****************************************************************************************/

#pragma once
#include "collect.h"
#include <cstdio>
#include "stack.h"
#include <stdint.h>

#define DEBUGE_MODE

#define set_stack_flag(x)		(void *) ((uintptr_t)x | (uintptr_t)1)
#define set_composite_flag(x)	(void *) ((uintptr_t)x | (uintptr_t)2)
#define set_both_flags(x)		(void *) ((uintptr_t)x | (uintptr_t)3)
#define is_stack_pointer(x)		(bool) ((uintptr_t)x & (uintptr_t)1)
#define is_composite_pointer(x)	(bool) ((uintptr_t)x & (uintptr_t)2)
#define clear_stack_flag(x)		(void *) ((uintptr_t)x & ~(uintptr_t)1)
#define clear_both_flags(x)		(void *) ((uintptr_t)x & ~(uintptr_t)3)

extern StackMap stack_ptr;
extern std::vector <size_t> offsets;
extern bool new_active; /**< global flag. False --- (out of gc_new) not to save offsets, true --- (in gc_new), save offsets */
extern bool no_active; /**< global flag. If true --- not to save offsets or set stack flag, because now is allocating an array in the heap */
extern size_t current_pointer_to_object; /**< used in offsets calculation */

struct Composite_pointer {
	void * pointer;
	void * base;
	size_t ref_count; //! count of referenses to this object
};

/**
* \class template smart pointer class gc_ptr
* \brief the class describes smart pointer
* \detailed template smart pointer class gc_ptr using to represent pointers and 
* 	override arithmetics and other operations on them.
*/
template <typename T> 
class gc_ptr {
private:
	void * ptr; /**< pointer on specified type*/

	void * get_base_ptr (void * ptr) {
		if (is_composite_pointer(ptr)) {
			return ((Composite_pointer *)(clear_both_flags(ptr)))->base;
		} else {
			return clear_stack_flag(ptr);
		}
	}

	T * get_ptr (void * pointer) const {
		if (is_composite_pointer(pointer)) {
			return reinterpret_cast<T *>( (((Composite_pointer *)(clear_both_flags(pointer)))->pointer));
		} else {
			return reinterpret_cast<T *>(clear_stack_flag(pointer));
		}
	}

	// only for gc_new
	gc_ptr (T* p) {
		#ifdef DEBUGE_MODE
			printf("gc_ptr(T* p) { %p\n", this);
		#endif
		ptr = (void *) p;
		if (no_active) {
			printf("\tno_active\n");
			return;
		}
		if (!new_active) {
			inc(this);
			ptr = set_stack_flag(ptr);
			#ifdef DEBUGE_MODE
				printf("\tstack\n");
			#endif
		} else if (is_heap_pointer(this)) {
			#ifdef DEBUGE_MODE
				printf("\theap\n");
			#endif
			assert(current_pointer_to_object != 0);
			offsets.push_back(reinterpret_cast <size_t> (this) - current_pointer_to_object);
		}
	}

public:
	/**	\fn construct gc_ptr()
		\brief setting ptr on null
	*/
	gc_ptr () {
		#ifdef DEBUGE_MODE
			printf("gc_ptr() { %p\n", this);
		#endif
		ptr = 0;
		if (no_active) {
			printf("\tno_active\n");
			return;
		}
		if (!new_active) {
			inc(this);
			ptr = set_stack_flag(ptr);
			#ifdef DEBUGE_MODE
				printf("\tstack\n");
			#endif
		} else if (is_heap_pointer(this)) {
			#ifdef DEBUGE_MODE
				printf("\theap\n");
			#endif
			assert(current_pointer_to_object != 0);
			offsets.push_back(reinterpret_cast <size_t> (this) - current_pointer_to_object);
		}
	}

	/**	\fn construct gc_ptr(const gc_ptr<int> &p)
		\brief setting pointer on given adress type of T 			
	*/
	gc_ptr (const gc_ptr <T> &p) {
		#ifdef DEBUGE_MODE
			printf("gc_ptr(const gc_ptr <T> &p) { %p\n", this);
		#endif
		ptr = clear_stack_flag(p.ptr); //< also set composite flag if necessary
		if (is_composite_pointer(p.ptr)) {
			((Composite_pointer *)(clear_both_flags(p.ptr)))->ref_count++;
		}
		if (no_active) {
			return;
		}
		if (!new_active) {
			inc(this);
			ptr = set_stack_flag(ptr);
		} else if (is_heap_pointer(this)) {
			assert(current_pointer_to_object != 0);
			offsets.push_back(reinterpret_cast <size_t> (this) - current_pointer_to_object);
		}
	}
	
	/**	\fn destructor gc_ptr()
		\brief delete current gc_ptr from special ptr_list  			
	*/
	~gc_ptr () {
		#ifdef DEBUGE_MODE
			printf("~gc_ptr: %p; ", this);
		#endif
		if (is_stack_pointer(ptr)) {
			#ifdef DEBUGE_MODE
				printf("stack ptr; ");
			#endif
			dec();
		}

		if (is_composite_pointer(ptr)) {
			#ifdef DEBUGE_MODE
				printf("composite ptr; ");
			#endif
			Composite_pointer * cp = (Composite_pointer *) clear_both_flags(ptr);
			assert(cp->ref_count > 0);
			if (cp->ref_count-- == 0) free(cp);
		}
		#ifdef DEBUGE_MODE
			printf("~gc_ptr: ends\n");
		#endif
	}

	/* reloaded operators for gc_ptrs objects*/
	T& operator* () const					{	return * get_ptr(ptr);									}
	T * operator-> () const 				{	return get_ptr(ptr);									}
	operator T * () const 					{	return get_ptr(ptr);									}
	T * get () const 						{	return get_ptr(ptr);									}
	T& operator[] (size_t index) const 		{	return (get_ptr(ptr))[index];							}
	T& operator[] (size_t index) 			{	return (get_ptr(ptr))[index];							}
	bool operator == (const gc_ptr <T> &a) 	{	return get_ptr(a.ptr) == get_ptr(ptr);					}
	bool operator != (const gc_ptr <T> &a) 	{	return get_ptr(a.ptr) != get_ptr(ptr);					}
	bool operator != (const T * a) 			{	return a != get_ptr(ptr);								}
	bool operator == (const T * a) 			{	return a == get_ptr(ptr);								}
	bool operator == (const int a) 			{	return a == reinterpret_cast<size_t> (get_ptr(ptr));	}
	bool operator != (const int a) 			{	return a != reinterpret_cast<size_t> (get_ptr(ptr));	}
	bool operator == (const long int a)		{	return a == reinterpret_cast<size_t> (get_ptr(ptr));	}
	bool operator != (const long int a)		{	return a != reinterpret_cast<size_t> (get_ptr(ptr));	}
	operator bool () const 					{	return get_ptr(ptr) != NULL;							}

	gc_ptr& operator = (const gc_ptr <T> &a) {
		#ifdef DEBUGE_MODE
			printf("gc_ptr& operator =\n");
		#endif
		if (is_composite_pointer(ptr)) {
			Composite_pointer * cp = (Composite_pointer *) clear_both_flags(ptr);
			assert(cp->ref_count > 0);
			if (cp->ref_count-- == 0) free(cp);
		}

		ptr = is_stack_pointer(ptr) ? set_stack_flag(a.ptr) : clear_stack_flag(a.ptr); // auto set composite flag
		if (is_composite_pointer(a.ptr)) {
			((Composite_pointer *)(clear_both_flags(a.ptr)))->ref_count++;
		}
		return *this;
	}

	/**	\fn an equivalent of the operator = (T * a)
		\brief
			use when you want to have a pointer not to the begin of some object;
			gc_ptr<R> base --- is a pointer to the begin of the object,
				uses only in GC to mark objects;
			T * pointer --- is a pointer somewere to the middle of the object,
				it is a pointer that points.
	*/
	template <typename R>
	void attach (T * pointer, gc_ptr<R> base) {
		#ifdef DEBUGE_MODE
			printf("attach %p \n", (void *)base);
		#endif
		if (is_composite_pointer(ptr)) {
			Composite_pointer * cp = (Composite_pointer *) clear_both_flags(ptr);
			assert(cp->ref_count > 0);
			if (cp->ref_count-- == 0) free(cp);
		}

		Composite_pointer * cp = (Composite_pointer *) malloc (sizeof(Composite_pointer)); {
			cp->base		= (void *) base;
			cp->pointer		= (void *) pointer;
			cp->ref_count	= 1;
		}
		ptr = is_stack_pointer(ptr) ? set_both_flags(cp) : set_composite_flag(cp);
	}

	void setNULL () {
		if (is_composite_pointer(ptr)) {
			Composite_pointer * cp = (Composite_pointer *) clear_both_flags(ptr);
			assert(cp->ref_count > 0);
			if (cp->ref_count-- == 0) free(cp);
		}

		ptr = 0;
	}

	template <class R, typename ... Types>
	friend
	gc_ptr<R> gc_new (Types ... , size_t);

	inline
	friend
	void * get_next_obj (void * v);

	void print () {
		printf("\ngc_ptr: %p\n", this);
		printf("\tpointer: %p\n", ptr);
		printf("\tis stack: %i\n", is_stack_pointer(ptr));
		printf("\tis composite: %i\n", is_composite_pointer(ptr));
		if (is_composite_pointer(ptr)) {
			Composite_pointer * cp = (Composite_pointer *)(clear_both_flags(ptr));
			printf("\tbase: %p pointer: %p ref_count: %zu\n", cp->base, cp->pointer, cp->ref_count);
		}
	}
};
