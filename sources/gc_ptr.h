/*************************************************************************************//**
		* File: gc_ptr.h
		* Description: This file describe smart pointer class gc_ptr
		* Detailed: gc_ptr --- is a template library pointer primitive
			use gc_ptr insted of regular C++ pointers for managed objects.
*****************************************************************************************/

#pragma once
#include <cstdio>
#include "stack.h"
#include <stdint.h>
#include <msmalloc.h>
#include "deref_roots.h"

// #define my_malloc no_space_malloc
// #define my_malloc space_based_malloc
// #define my_malloc timed_malloc
#define my_malloc stupid_malloc
// #define my_malloc malloc

#define set_stack_flag(x)		(void *)	((uintptr_t)x | (uintptr_t)1)
#define set_composite_flag(x)	(void *)	((uintptr_t)x | (uintptr_t)2)
#define set_both_flags(x)		(void *)	((uintptr_t)x | (uintptr_t)3)
#define is_stack_pointer(x)		(bool)		((uintptr_t)x & (uintptr_t)1)
#define is_composite_pointer(x)	(bool)		((uintptr_t)x & (uintptr_t)2)
#define clear_stack_flag(x)		(void *)	((uintptr_t)x & ~(uintptr_t)1)
#define clear_both_flags(x)		(void *)	((uintptr_t)x & ~(uintptr_t)3)

extern std::vector <size_t> offsets;
extern bool new_active;	/**< global flag. False --- (out of gc_new) not to save offsets, true --- (in gc_new), save offsets */
extern bool no_active;	/**< global flag. If true --- not to save offsets or set stack flag, because now is allocating an array in the heap */
extern size_t current_pointer_to_object;	/**< used in offsets calculation */

/**
* @class Composite_pointer
* @brief represents tertiary pointer level;
	uses for "creation pointers inside object" suppport.
*/
struct Composite_pointer {
	void * pointer;	//! pointer somewere inside object
	void * base;	//! pointer to the comprehensive object begin
	size_t ref_count;	//! referense counter
};

/**
* @class base_meta
* @brief object meta base class
* @detailed realizes base object meta class (like interface for eatch object meta)
*/
class base_meta {
public:
	void *shell;	/**< pointer on the box(meta info struct for storing offsets) of object */
	void * ptrptr;	/**< pointer to the real object begin */
	size_t size;	/**< size of object */
	virtual void del_ptr () = 0;	/**< delete meta-ptr */
	virtual void* get_begin () = 0;	/**< get begin of object (pointer on meta)*/
};

/**
* @class meta
* @brief template class; realizes specific meta for eatch object;
* @detailed it creates in gc_new and it it stored directly with (right before) the
	allocated object.
*/
template <class T>
class meta : public base_meta {
public:
	T* ptr;	/**< "typed" pointer to the real object begin */

	/// virtual del_ptr function from base_meta realization
	void del_ptr (void) {
		dprintf("in del_ptr\n");
		if (size == 1) {
			((T*)ptr)->~T();
		} else {
			for (size_t i = 0; i < size; i++)
				((T*)ptr)[i].~T();
		}
	}

	/// virtual get_begin function from base_meta realization
	void* get_begin (void) {
		dprintf("in get_begin\n");
		return reinterpret_cast <void*> (this);
	}
};


/**
* @class template smart pointer class gc_ptr
* @brief the class describes library pointer primitive
* @detailed template smart pointer class gc_ptr using to represent secondary pointer level and 
* 	overrides arithmetics and other operations on them.
*/
template <typename T> 
class gc_ptr {
private:
	void * ptr; /**< pointer directly on specified managed object */

	/**
	* @function get_base_ptr
	* @param ptr is a pointer directly on the managed object
	* @return pointer to the object meta (look at base_meta and class_meta in gc_new.h)
	*/
	void * get_base_ptr (void * ptr) const {
		dprintf("get_base_ptr\n");
		if (is_composite_pointer(ptr)) {
			return ((Composite_pointer *)(clear_both_flags(ptr)))->base;
		} else {
			return clear_stack_flag(ptr);
		}
	}

	/**
	* @function ptr
	* @param ptr some pointer
	* @return some correctly aligned pointer (cleared from flags)
	*/
	T * get_ptr (void * pointer) const {
		dprintf("T * get_ptr\n");
		if (is_composite_pointer(pointer)) {
			return reinterpret_cast<T *>( (((Composite_pointer *)(clear_both_flags(pointer)))->pointer));
		} else {
			return reinterpret_cast<T *>(clear_stack_flag(pointer));
		}
	}

	/**
	* @constructor
	* @detailed only for gc_new, because it is unsafe to create
		gc_ptr from some pointer outside the gc_new
		following the fact that correct meta might be before the obgect in memory;
		only gc_new can garanty this.
	* @param p pointer manafed object
	*/
	gc_ptr (T* p) {
		dprintf("gc_ptr(T* p) { %p\n", this);
		ptr = (void *) p;
		if (!new_active) {
			StackMap * stack_ptr = StackMap::getInstance();
			stack_ptr->register_stack_root(this);
			ptr = set_stack_flag(ptr);
			dprintf("\tstack\n");
		} else if (is_heap_pointer(this)) {
			if (no_active) {
				dprintf("\tno_active\n");
				return;
			}
			dprintf("\theap\n");
			assert(current_pointer_to_object != 0);
			offsets.push_back(reinterpret_cast <size_t> (this) - current_pointer_to_object);
		}
	}

public:
	/**
	* @return is it automatic object (in program stack) or not (i.e. in heap)
	*/
	bool is_stack_ptr () {
		dprintf("get_stack_ptr\n");
		return is_stack_pointer(ptr);
	}

	/**
	* @constructor default constructor
	* @detailed sets ptr pointer on NULL
	*/
	gc_ptr () {
		dprintf("gc_ptr() { %p\n", this);
		ptr = 0;
		if (!new_active) {
			StackMap * stack_ptr = StackMap::getInstance();
			stack_ptr->register_stack_root(this);
			ptr = set_stack_flag(ptr);
			dprintf("\tstack\n");
		} else if (is_heap_pointer(this)) {
			if (no_active) {
				dprintf("\tno_active\n");
				return;
			}
			dprintf("\theap; offsets:push_back\n");
			assert(current_pointer_to_object != 0);
			offsets.push_back(reinterpret_cast <size_t> (this) - current_pointer_to_object);
		}
		dprintf("end gc_ptr()\n");
	}

	/**
	* @constructor copy constructor
	* @param p is a gc_ptr to be copied
	*/
	gc_ptr (const gc_ptr <T> &p) {
		dprintf("gc_ptr (const ...)\n");
		ptr = clear_stack_flag(p.ptr); //< also set composite flag if necessary
		if (is_composite_pointer(p.ptr)) {
			((Composite_pointer *)(clear_both_flags(p.ptr)))->ref_count++;
		}
		if (!new_active) {
			dprintf("\tstack pointer\n");
			StackMap * stack_ptr = StackMap::getInstance();
			stack_ptr->register_stack_root(this);
			ptr = set_stack_flag(ptr);
		} else if (is_heap_pointer(this)) {
			if (no_active) {
				dprintf("\tno_active\n");
				return;
			}
			dprintf("\theap pointer\n");
			assert(current_pointer_to_object != 0);
			offsets.push_back(reinterpret_cast <size_t> (this) - current_pointer_to_object);
		}
	}

	/**
	* @constructor move constructor ?!?
	* @param p is gc_ptr to be moved
	*/
	gc_ptr (gc_ptr <T> &&p) {
		ptr = clear_stack_flag(p.ptr);
	}
	
	/**
	* @destructor
	* @detailed gc_ptr dectructor;
		removes itself from root stack if neccesary;
		clear composite pointer if neccesary.
	*/
	~gc_ptr () {
		dprintf("~gc_ptr: %p; ", this);
		if (is_stack_pointer(ptr)) {
			dprintf("~gc_ptr -> delete stack root: %p\n", this);
			StackMap * stack_ptr = StackMap::getInstance();
			stack_ptr->delete_stack_root(this);
		}
		if (is_composite_pointer(ptr)) {
			dprintf("composite ptr; ");
			Composite_pointer * cp = (Composite_pointer *) clear_both_flags(ptr);
			assert(cp->ref_count > 0);
			if (cp->ref_count-- == 0) free(cp);
		}
		dprintf("~gc_ptr: ends\n");
	}

	/* reloaded operators for gc_ptr's objects */
	T& operator* () const					{	return * get_ptr(ptr);									}
	T* operator->() const {
		T *p = get_ptr(ptr);
		register_dereferenced_root(p, ((meta<T>*)get_base_ptr(p))->size * sizeof(T));
		return p;
	}
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
		dprintf("gc_ptr& operator =\n");
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
		dprintf("attach %p \n", (void *)base);
		if (is_composite_pointer(ptr)) {
			Composite_pointer * cp = (Composite_pointer *) clear_both_flags(ptr);
			assert(cp->ref_count > 0);
			if (cp->ref_count-- == 0) free(cp);
		}

		Composite_pointer * cp = (Composite_pointer *) my_malloc (sizeof(Composite_pointer)); {//malloc (sizeof(Composite_pointer)); {
			cp->base		= (void *) base;
			cp->pointer		= (void *) pointer;
			cp->ref_count	= 1;
		}
		ptr = is_stack_pointer(ptr) ? set_both_flags(cp) : set_composite_flag(cp);
	}

	/**	\fn nullify function
		\brief nullifies current object
	*/
	void setNULL () {
		dprintf("setNULL\n");
		if (is_composite_pointer(ptr)) {
			Composite_pointer * cp = (Composite_pointer *) clear_both_flags(ptr);
			assert(cp->ref_count > 0);
			if (cp->ref_count-- == 0) free(cp);
		}
		ptr = 0;
	}

	/** look at gc_new.h */
	template <class R, typename ... Types>
	friend
	gc_ptr<R> gc_new (Types ... , size_t);

	/** look at go.h */
	inline
	friend
	void * get_next_obj (void * v);

	/**	\fn print
		\brief prints gc_ptr stamp
	*/
	void print (void) {
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
