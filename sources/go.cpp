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
#include <stack>

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
#ifdef DEBUGE_MODE
	printf("go.cpp: get_ptr\n"); fflush(stdout);
#endif
	if (is_composite_pointer(ptr)) {
	#ifdef DEBUGE_MODE
			printf(" %p comp %p \n ", ptr, ((Composite_pointer *)(clear_both_flags(ptr)))->base);
	#endif
		mark(clear_both_flags(ptr));
		return ((Composite_pointer *)(clear_both_flags(ptr)))->base;
	} else {
	#ifdef DEBUGE_MODE
			printf(" %p !comp %p\n ", ptr, clear_stack_flag(ptr));
	#endif
		return clear_stack_flag(ptr);
	}
}

/**
* @function get_next_obj
* @return pointer (void *) on the object on that root or gc_ptr "v" points;
	in case v is invalide pointer to the gc_ptr "v" returns NULL.
* @param v --- pointer (like gc_ptr)
*/
inline void * get_next_obj(void * v) {  /* get the next object*/
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

/**
* @function go
* @detailed recursive traversing function (implements mark gc phase);
	marks object as alive and folloeing its object meta,
	starting from pointer "v", it walk around reached objects graph
	by calling itself (function go) for all objects object "v" points to;
* @return nothing
* @param v --- is a current traversing object (in first call --- roots and fake roots)
*/
void go (void * pointer) {
#ifdef DEBUGE_MODE
	printf("\nin go\n"); fflush(stdout);
#endif
	if (!pointer || !is_heap_pointer(pointer)) {
	#ifdef DEBUGE_MODE
		printf("\nreturn! --- NULL or NON-heap pointer %p\n", pointer);	fflush(stdout);
	#endif
		return;
	}

	std::stack<void *> vertices;
	vertices.push(pointer);

	while (vertices.size() != 0) {
		void * v = vertices.top();	vertices.pop();

		if (v == NULL || !is_heap_pointer(v)) {
		#ifdef DEBUGE_MODE
			printf(" %p is not a heap pointer\n ", v);
		#endif
			continue;
		}
		
		base_meta* bm = get_meta_inf(v); /* get metainformation from object*/
	#ifdef DEBUGE_MODE
		printf("base_meta %p\n", bm); fflush(stdout);
		if (!is_heap_pointer(bm)) {
			printf("NOT HEAP bm \n"); fflush(stdout);
		}
	#endif
		if (get_mark(bm) != 0) {// || get_mark(v) != 0) { /* if marked --- return*/
		#ifdef DEBUGE_MODE
			printf("%p %p already marked\n ", bm, v);
			fflush(stdout);
		#endif
			continue;
		}
		void *shell = bm->shell;  /* saving ponter on meta object in shell */
		BLOCK_TAG* tag = (BLOCK_TAG *) shell; /* store shell in tag */
		if (tag->model == 0) {  /* checking tag modell, correct if rang 1,2,4*/
			throw tag;
		}

		mark(bm);
	#ifdef DEBUGE_MODE
		live_object_count++;
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
								vertices.push(get_next_obj(p));
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
						#ifdef DEBUGE_MODE
							printf(" %i ", i);
						#endif
							void * meta = tag->ptr;
							void * this_offsets = (char *)meta + sizeof(BLOCK_TAG) + sizeof(size_t);
							size_t n = *(size_t *)((char *)meta + sizeof(BLOCK_TAG));
							for (size_t i = 0; i < n; i++) {
								void *p = (char*)poin + (*((POINTER_DESCR *)this_offsets)).offset;
								if (p) {
									vertices.push(get_next_obj(p));
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
			printf("Error of data representation!\nAttention, null - value of tag-model\n");
			printf("tag value:%p\n", tag);
			printf("tag-model value: %u\n", tag->model);
			printf("tag-size value: %lu\n", tag->size);
			fflush(stdout);
			exit(1);
		} catch(...) {
			printf("UNEXPECTED ERROR!!! CHECK tag->mbit");
			fflush(stdout);
			exit(1);
		}
	}
}

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
#ifdef DEBUGE_MODE
	printf("go.cpp: mark_and_sweep\n"); fflush(stdout);
#endif
	printf("mark and sweep!\nbefore:");
	printDlMallocInfo();

	mark_fake_roots();

#ifdef DEBUGE_MODE
	live_object_count = 0;
	printf("mark\n"); fflush(stdout);
	int i = 0;
	printf("roots: ");
#endif
	// iterate root stack and call traversing function go
	StackMap * stack_ptr = StackMap::getInstance();
	for(Iterator root = stack_ptr->begin(); root <= stack_ptr->end(); root++) {/* walk through all roots*/
		go(get_next_obj(*root)); /* mark all available objects with mbit = 1*/
	#ifdef DEBUGE_MODE
		i++;
		printf("root %p ", get_next_obj(*root));
	#endif
	}
#ifdef DEBUGE_MODE
	printf("\nroot count = %i; live_object_count = %zu\n", i, live_object_count);
	live_object_count = 0;
	printf("sweep"); fflush(stdout);
#endif
	// call sweep function (look at msmalloc)
	sweep();
	printf("after: "); printDlMallocInfo(); fflush(stdout);
	// printf("after m&s "); printDlMallocInfo(); fflush(stdout);
}
