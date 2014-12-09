/****************************************************************************************
		* File: go.cpp
		* Description: functions in this file realizes heap traversing (mark phase)
			and mark and sweep function
*****************************************************************************************/

#include "taginfo.h"
#include "gc_new.h"
#include "stack.h"
#include "fake_roots.h"
#include <stdint.h>
#include "go.h"

#ifdef DEBUGE_MODE
	#undef DEBUGE_MODE
#endif
// #define DEBUGE_MODE

extern int nesting_level;

#ifdef DEBUGE_MODE
	size_t live_object_count = 0;
#endif

/**
* @function get_ptr
* @return pointer that is cleared from glags was setted in gc_ptr
* @param ptr --- some pointer (really is a pointer to th managed object)
*/
void * get_ptr (void * ptr) {
#ifndef DEBUGE_MODE
	if (is_composite_pointer(ptr)) {
		mark(clear_both_flags(ptr));
		return ((Composite_pointer *)(clear_both_flags(ptr)))->base;
	} else {
		return clear_stack_flag(ptr);
	}
#else
	printf("go.cpp: get_ptr\n"); fflush(stdout);
	if (is_composite_pointer(ptr)) {
		printf(" %p comp %p \n ", ptr, ((Composite_pointer *)(clear_both_flags(ptr)))->base);
		mark(clear_both_flags(ptr));
		return ((Composite_pointer *)(clear_both_flags(ptr)))->base;
	} else {
		printf(" %p !comp %p\n ", ptr, clear_stack_flag(ptr));
		return clear_stack_flag(ptr);
	}
#endif
}

/**
* @function get_next_obj
* @return pointer (void *) on the object on that root or gc_ptr "v" points;
	in case v is invalide pointer to the gc_ptr "v" returns NULL.
* @param v --- pointer (like gc_ptr)
*/
void * get_next_obj(void * v) {  /* get the next object*/
#ifdef DEBUGE_MODE
	printf(" get_next_obj %p ", v); fflush(stdout);
#endif
	void * res = reinterpret_cast <void*> (*((size_t *)v));
	if (res == NULL)
		return NULL;
	
#ifdef DEBUGE_MODE
	printf(" res %p\n ", res); fflush(stdout);
#endif
	return clear_both_flags(res) == NULL ? NULL : get_ptr(res);
}

/**
* @function get_meta_inf
* @return object of class base_meta (object meta) that is ctored directly with managed object
* @param v --- pointer to the managed object
* TODO: it can throw exception in "v"'s incorrect pointer case
*/
inline base_meta * get_meta_inf (void * v) {  /*!< get the block with meta_inf*/
	return reinterpret_cast <base_meta *> ( *(reinterpret_cast <size_t *> (reinterpret_cast <size_t>(v) - sizeof(base_meta *))));
}

void * to_get_meta_inf (void * v) {  /*!< get the block with meta_inf*/
	return reinterpret_cast <void *> (reinterpret_cast <base_meta*>(v) + 1);
}

/**	ifs stack frous up to max_stack_size
*	then overflow case will be
*/
#define max_stack_size 10
struct Stack {
	struct StackEl {
		void * pointer;
		StackEl * prev;
	};
	StackEl * top;
	int size;

	int push (void * new_element) {
		StackEl * st = (StackEl *) malloc (sizeof(StackEl));
		if (!st || size >= max_stack_size) {
			// fails to allocate memory
			return 1;
		}
		st->prev = top;
		st->pointer = new_element;
		top = st;
		size++;
		return 0;
	}

	void * pop () {
		if (!top) {
			return NULL;
		}
		void * res = top->pointer;
		StackEl * st = top;
		top = top->prev;
		free(st);
		return res;
	}

	bool is_empty () {
		return top == NULL;
	}
};

/**
* @function go
* @detailed recursive traversing function (implements mark gc phase);
	marks object as alive and folloeing its object meta,
	starting from pointer "v", it walk around reached objects graph
	by calling itself (function go) for all objects object "v" points to;
* @return nothing
* @param v --- is a current traversing object (in first call --- roots and fake roots)
*/
#ifndef DEBUGE_MODE
int go (void * pointer) {
	if (!pointer || !is_heap_pointer(pointer)) {
		return 0;
	}
	Stack * vertices = (Stack *)malloc(sizeof(Stack));
	vertices->top = NULL;
	vertices->size = 0;
	vertices->push(pointer);
	bool stack_overflow = false;

	while (!vertices->is_empty()) {
		void * v = vertices->pop();
		if (v == NULL || !is_heap_pointer(v)) {
			continue;
		}
		base_meta* bm = get_meta_inf(v); /* get metainformation from object*/
		void *shell = bm->shell;  /* saving ponter on meta object in shell */
		BLOCK_TAG* tag = (BLOCK_TAG *) shell; /* store shell in tag */

		if (!get_mark(bm)) {
			mark(bm);
		}
		try {
			switch (tag->model) {
				case 1: {  /* boxed object */
						size_t n = *(size_t *)((char *)shell + sizeof(BLOCK_TAG));  /* count of offsets*/
						void * this_offsets = (char *)shell + sizeof(BLOCK_TAG) + sizeof(size_t);  /* get first offset*/
						for (size_t i = 0; i < n; i++) {  /* walk throught offsets*/
							void *p = (char*)v + (*((POINTER_DESCR *)this_offsets)).offset;  /* get object by offset*/
							if (p) {
								void * next_vertice = get_next_obj(p);
								if (next_vertice && get_mark(get_meta_inf(next_vertice)) == 0) {
									if (vertices->push(next_vertice)) { //i.e. fails to allocate memory
										stack_overflow = true;
									}
								}
							}
							this_offsets = (char *)this_offsets + sizeof(POINTER_DESCR);   /* get next pointer in this obj*/
						}
					}
					break;
				case 2:  /* simple obj*/
					break;
				case 3: {  /* boxed array*/
						void * poin = v;
						size_t sizeType = tag->num_of_el, sizeArray = tag->size;
						for (size_t i = 0; i < sizeArray; i++, poin = (void *)((char *)poin + sizeType)) {
							void * meta = tag->ptr;
							void * this_offsets = (char *)meta + sizeof(BLOCK_TAG) + sizeof(size_t);
							size_t n = *(size_t *)((char *)meta + sizeof(BLOCK_TAG));
							for (size_t i = 0; i < n; i++) {
								void *p = (char*)poin + (*((POINTER_DESCR *)this_offsets)).offset;
								if (p) {
									void * next_vertice = get_next_obj(p);
									if (next_vertice && get_mark(get_meta_inf(next_vertice)) == 0) {
										if (vertices->push(next_vertice)) { //i.e. fails to allocate memory
											stack_overflow = true;
										}
									}
								}
								this_offsets = (char *)this_offsets + sizeof(POINTER_DESCR);
							}
						}
					}
					break;
				case 4:  /* unboxed_array*/
					break;
				default:
					throw tag;
					break;
			}
		} catch(BLOCK_TAG* tag) {
			printf("FUNCTION go : wrong tag!\n"); fflush(stdout);
			exit(1);
		} catch(...) {
			printf("UNEXPECTED ERROR!!! CHECK tag->mbit"); fflush(stdout);
			exit(1);
		}
	}
	free(vertices);
	return stack_overflow;
}
#else
int go (void * pointer) {
	printf("\nin go %p\n", pointer); fflush(stdout);
	
	if (!pointer || !is_heap_pointer(pointer)) {
	#ifdef DEBUGE_MODE
		printf("\nreturn! --- NULL or NON-heap pointer %p\n", pointer);	fflush(stdout);
	#endif
		return 0;
	}
	Stack * vertices = (Stack *)malloc(sizeof(Stack));
	vertices->top = NULL;
	vertices->size = 0;
	vertices->push(pointer);
	bool stack_overflow = false;

	while (!vertices->is_empty()) {
		void * v = vertices->pop();
		if (v == NULL || !is_heap_pointer(v)) {
		#ifdef DEBUGE_MODE
			printf(" %p is not a heap pointer\n ", v);
		#endif
			continue;
		}
		base_meta* bm = get_meta_inf(v); /* get metainformation from object*/
		void *shell = bm->shell;  /* saving ponter on meta object in shell */
		BLOCK_TAG* tag = (BLOCK_TAG *) shell; /* store shell in tag */

		if (!get_mark(bm)) {
		#ifdef DEBUGE_MODE
			printf("%p %p already marked\n ", bm, v); fflush(stdout);
		#endif
			mark(bm);
		#ifdef DEBUGE_MODE
			live_object_count++;
		#endif
		}
	#ifdef DEBUGE_MODE
		printf("go: alive: %p %p\n", bm, v);
		printf("mark object %p\n", bm);
		printf("tag->model:%i object: %p\n", tag->model, bm);
		fflush(stdout);
	#endif
		try {
			switch (tag->model) {
				case 1: {  /* boxed object */
						size_t n = *(size_t *)((char *)shell + sizeof(BLOCK_TAG));  /* count of offsets*/
						void * this_offsets = (char *)shell + sizeof(BLOCK_TAG) + sizeof(size_t);  /* get first offset*/
					#ifdef DEBUGE_MODE
						printf("offset count: %zu\n", n);
					#endif
						for (size_t i = 0; i < n; i++) {  /* walk throught offsets*/
							void *p = (char*)v + (*((POINTER_DESCR *)this_offsets)).offset;  /* get object by offset*/
						#ifdef DEBUGE_MODE
							printf("offset: %zu %p\n", (*((POINTER_DESCR *)this_offsets)).offset, bm);
						#endif
							if (p) {
								void * next_vertice = get_next_obj(p);
								if (next_vertice && get_mark(get_meta_inf(next_vertice)) == 0) {
									printf("go : tag 1 : push : %p %i %i\n", next_vertice, get_mark(next_vertice),
										get_mark(get_meta_inf(next_vertice))); fflush(stdout);
									if (vertices->push(next_vertice)) { //i.e. fails to allocate memory
										// TODO: and ?
										printf("FUNCTION Go : tag : case1 : out of memory\n"); fflush(stdout);
										// exit(1);
										stack_overflow = true;
									}
								} else {
									printf("go : tag 1 : NOT push : %p ;mark = %i \n", next_vertice, get_mark(next_vertice)); fflush(stdout);
									// exit(1);
								}
							}
							this_offsets = (char *)this_offsets + sizeof(POINTER_DESCR);   /* get next pointer in this obj*/
						}
					}
					break;
				case 2:  /* simple obj*/
					break;
				case 3: {  /* boxed array*/
						void * poin = v;
						size_t sizeType = tag->num_of_el, sizeArray = tag->size;
						for (size_t i = 0; i < sizeArray; i++, poin = (void *)((char *)poin + sizeType)) {
							void * meta = tag->ptr;
							void * this_offsets = (char *)meta + sizeof(BLOCK_TAG) + sizeof(size_t);
							size_t n = *(size_t *)((char *)meta + sizeof(BLOCK_TAG));
							for (size_t i = 0; i < n; i++) {
							#ifdef DEBUGE_MODE
								printf(" %i ", i);
							#endif
								void *p = (char*)poin + (*((POINTER_DESCR *)this_offsets)).offset;
								if (p) {
									void * next_vertice = get_next_obj(p);
									if (next_vertice && get_mark(get_meta_inf(next_vertice)) == 0) {
										if (vertices->push(next_vertice)) { //i.e. fails to allocate memory
											// TODO: and ?
											printf("FUNCTION Go : tag : case3 : out of memory\n"); fflush(stdout);
											// exit(1);
											stack_overflow = true;
										}
									}
								}
								this_offsets = (char *)this_offsets + sizeof(POINTER_DESCR);
							}
						}
					}
					break;
				case 4:  /* unboxed_array*/
					break;
				default:
					throw tag;
					break;
			}
		} catch(BLOCK_TAG* tag) {
			printf("FUNCTION go : wrong tag!\n"); fflush(stdout);
			exit(1);
		} catch(...) {
			printf("UNEXPECTED ERROR!!! CHECK tag->mbit"); fflush(stdout);
			exit(1);
		}
	}
	free(vertices);
	return stack_overflow;
}
#endif

/**
* @function gc
* @detailed forse garbage collection call for malloc's from msmalloc
* @return 0 in normal case; 1 in unsafe point case (nesting_level != 0)
*/
int gc () {
#ifdef DEBUGE_MODE
	printf("in gc, %i ", nesting_level); fflush(stdout);
#endif
	if (nesting_level != 0) {
		return 1;
	}
#ifdef DEBUGE_MODE
	printf("gc: mark_and_sweep\n");
	fflush(stdout);
#endif
	mark_and_sweep();
#ifdef DEBUGE_MODE
	printf("\n"); fflush(stdout);
#endif
	return 0;
}

/**
* @override operator delete 
* @detailed C++ standart garanties destructor call,
	it is true in our terms iff p --- is valid non-managed object pointer;
* @param p --- pointer on object to be freed
*/
void operator delete (void * p) {
// TODO: needs check on managed; if true --- not to call free, otherwise call free
// TODO: then to call
}

/**
* @function gc_delete
* @detailed gc delete function
* @param chunk --- pointer on chunk to be freed
*/
void gc_delete (void * chunk) {
#ifdef DEBUGE_MODE
	printf("go.cpp: gc_delete\n"); fflush(stdout);
#endif
	free(chunk);
}

/**
* @function mark_and_sweep
* @detailed implements mark and sweep stop the world algorithm
*/
void mark_and_sweep () {
#ifndef DEBUGE_MODE
	printf("mark and sweep!\nbefore:");	printDlMallocInfo(); fflush(stdout);
	mark_fake_roots();

	// iterate root stack and call traversing function go
	StackMap * stack_ptr = StackMap::getInstance();
	bool stack_overflow = false;
	for(Iterator root = stack_ptr->begin(); root <= stack_ptr->end(); root++) {/* walk through all roots*/
		stack_overflow |= go(get_next_obj(*root)); /* mark all available objects with mbit = 1*/
	}
	while (stack_overflow) {
		stack_overflow = mark_after_overflow();
	}
	
	// call sweep function (look at msmalloc)
	sweep();
	printf("after: "); printDlMallocInfo(); fflush(stdout);
#else
	printf("go.cpp: mark_and_sweep\n"); fflush(stdout);
	printf("mark and sweep!\nbefore:");	printDlMallocInfo(); fflush(stdout);
	mark_fake_roots();

	live_object_count = 0;
	printf("mark\n"); fflush(stdout);
	int i = 0;
	printf("roots: ");

	// iterate root stack and call traversing function go
	StackMap * stack_ptr = StackMap::getInstance();
	bool stack_overflow = false;
	for(Iterator root = stack_ptr->begin(); root <= stack_ptr->end(); root++) {/* walk through all roots*/
		stack_overflow |= go(get_next_obj(*root)); /* mark all available objects with mbit = 1*/
		i++;
		printf("root %p ", get_next_obj(*root));
	}
	printf("\nroot count = %i; live_object_count = %zu\n", i, live_object_count);
	live_object_count = 0;
	printf("sweep"); fflush(stdout);
	int over_count = 0;
	while (stack_overflow) {
		printf("mark_after_overflow\n"); fflush(stdout);
		stack_overflow = mark_after_overflow();
		over_count++;
	}
	printf("over_count = %i\n", over_count);
	// call sweep function (look at msmalloc)
	sweep();
	printf("after: "); printDlMallocInfo(); fflush(stdout);
#endif
}
