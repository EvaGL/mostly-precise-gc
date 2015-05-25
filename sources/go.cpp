/****************************************************************************************
		* File: go.cpp
		* Description: functions in this file realizes heap traversing (mark phase)
			and mark and sweep function
*****************************************************************************************/

#include "taginfo.h"
#include "gc_new.h"
#include "fake_roots.h"
#include "threading.h"
#ifdef DEBUGE_MODE
	size_t live_object_count = 0;
#endif

/**
* @function get_ptr
* @return pointer that is cleared from glags was setted in gc_ptr
* @param ptr --- some pointer (really is a pointer to th managed object)
*/
void * get_ptr (void * ptr) {
	if (is_composite_pointer(ptr)) {
		dprintf(" %p comp %p \n ", ptr, ((Composite_pointer *)(clear_both_flags(ptr)))->base);
		mark(clear_both_flags(ptr));
		return ((Composite_pointer *)(clear_both_flags(ptr)))->base;
	} else {
		dprintf(" %p !comp %p\n ", ptr, clear_stack_flag(ptr));
		return clear_stack_flag(ptr);
	}
}

void * move_ptr(void* ptr, void* value) {
	if (is_composite_pointer(ptr)) {
		((Composite_pointer*)clear_both_flags(ptr))->base = value;
		return ptr;
	} else {
		return restore_flags(value, get_both_flags(ptr));
	}
}

/**
* @function get_next_obj
* @return pointer (void *) on the object on that root or gc_ptr "v" points;
	in case v is invalide pointer to the gc_ptr "v" returns NULL.
* @param v --- pointer (like gc_ptr)
*/
void * get_next_obj(void * v) {  /* get the next object*/
	dprintf(" get_next_obj %p ", v); fflush(stdout);
	void * res = reinterpret_cast <void*> (*((size_t *)v));
	if (res == NULL)
		return NULL;

	dprintf(" res %p\n ", res); fflush(stdout);
	return clear_both_flags(res) == NULL ? NULL : get_ptr(res);
}

/**
* @function get_meta_inf
* @return object of class base_meta (object meta) that is ctored directly with managed object
* @param v --- pointer to the managed object
* TODO: it can throw exception in "v"'s incorrect pointer case
*/
base_meta * get_meta_inf (void * v) {  /*!< get the block with meta_inf*/
	return (base_meta*)v - 1;
}

void * to_get_meta_inf (void * v) {  /*!< get the block with meta_inf*/
	return reinterpret_cast <void *> ((base_meta*)(v) + 1);
}

/**	ifs stack frous up to max_stack_size
*	then overflow case will be
*/
#define max_stack_size 10000
void* stack[max_stack_size];
size_t stack_size;
	int push (void * new_element) {
		if (stack_size >= max_stack_size) {
			return 1;
		}
		stack[stack_size++] = new_element;
		return 0;
	}

	void * pop () {
		if (stack_size == 0) {
			return NULL;
		}
		return stack[--stack_size];
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
int go (void * pointer, bool pin_root) {
	dprintf("\nin go %p\n", pointer);

	if (!pointer || !is_heap_pointer(pointer)) {
		dprintf("\nreturn! --- NULL or NON-heap pointer %p\n", pointer);
		return 0;
	}
	if (pin_root) {
		pin(get_meta_inf(pointer));
	}

	stack_size = 1;
	stack[0] = pointer;

	bool stack_overflow = false;

	while (stack_size != 0) {
		void * v = pop();
		if (v == NULL || !is_heap_pointer(v)) {
			dprintf(" %p is not a heap pointer\n ", v);
			continue;
		}
		base_meta* bm = get_meta_inf(v); /* get metainformation from object*/
		void *shell = bm->shell;  /* saving ponter on meta object in shell */
		BLOCK_TAG* tag = (BLOCK_TAG *) shell; /* store shell in tag */

		if (!get_mark(bm)) {
			dprintf("go: alive: bm:%p v:%p tag->model:%i\n", bm, v, tag->model);
			mark(bm);
		#ifdef DEBUGE_MODE
			live_object_count++;
		#endif
		} else {
			dprintf("%p %p already marked\n ", bm, v);
		}
		try {
			switch (tag->model) {
				case 1: {  /* boxed object */
						size_t n = *(size_t *)((char *)shell + sizeof(BLOCK_TAG));  /* count of offsets*/
						void * this_offsets = (char *)shell + sizeof(BLOCK_TAG) + sizeof(size_t);  /* get first offset*/
						dprintf("offset count: %zu\n", n);
						for (size_t i = 0; i < n; i++) {  /* walk throught offsets*/
							void *p = (char*)v + (*((POINTER_DESCR *)this_offsets)).offset;  /* get object by offset*/
							dprintf("offset: %zu %p\n", (*((POINTER_DESCR *)this_offsets)).offset, bm);
							if (p) {
								void * next_vertice = get_next_obj(p);
								if (next_vertice && get_mark(get_meta_inf(next_vertice)) == 0) {
									dprintf("go : tag 1 : push : %p %i %i\n", next_vertice, get_mark(next_vertice),
										get_mark(get_meta_inf(next_vertice)));
									if (push(next_vertice)) { //i.e. fails to allocate memory
										dprintf("go : tag 1 : NOT push : %p ;mark = %i : out of memory\n", next_vertice,
											get_mark(next_vertice));
										stack_overflow = true;
									} else {
										dprintf("FUNCTION Go : tag : case1 : offset is pushed\n");
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
							for (size_t j = 0; j < n; j++) {
								dprintf(" %i ", i);
								void *p = (char*)poin + (*((POINTER_DESCR *)this_offsets)).offset;
								if (p) {
									void * next_vertice = get_next_obj(p);
									if (next_vertice && get_mark(get_meta_inf(next_vertice)) == 0) {
										if (push(next_vertice)) { //i.e. fails to allocate memory
											dprintf("go : tag 3 : NOT push : %p ;mark = %i : out of memory\n", next_vertice,
												get_mark(next_vertice));
											stack_overflow = true;
										} else {
											dprintf("FUNCTION Go : tag : case3 : offset is pushed\n");
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
			dprintf("FUNCTION go : wrong tag!\n");
			exit(1);
		} catch(...) {
			dprintf("UNEXPECTED ERROR!!! CHECK tag->mbit");
			exit(1);
		}
	}
	// free(vertices);
	return stack_overflow;
}

void clean_deref_roots();

/**
* @function gc
* @detailed forse garbage collection call for malloc's from msmalloc
* @return 0 in normal case; 1 in unsafe point case (nesting_level != 0)
*/
int gc (bool full) {
	dprintf("gc: mark_and_sweep\n");
	pthread_mutex_lock(&gc_mutex);
		thread_handler* handler = get_thread_handler(pthread_self());
		if (handler->tlflags->nesting_level != 0) {
			pthread_mutex_unlock(&gc_mutex);
			return 1;
		}
		enter_safepoint(handler);
		handler->stack_top = __builtin_frame_address(0);
		if (!gc_thread) {
			gc_thread = handler;
			dprintf("thread %d is garbage collector\n", handler->thread);
			if (full) {
				mark_and_sweep();
			} else {
				clean_deref_roots();
			}
			gc_thread = nullptr;
			exit_safepoint(handler);
			pthread_cond_broadcast(&gc_is_finished);
		} else {
			dprintf("Thread %d reached safepoint\n", handler->thread);\
			pthread_cond_signal(&safepoint_reached);
			pthread_cond_wait(&gc_is_finished, &gc_mutex);
			exit_safepoint(handler);
		}
	pthread_mutex_unlock(&gc_mutex);
	return 0;
}

/**
* @override operator delete 
* @detailed C++ standart garanties destructor call,
	it is true in our terms iff p --- is valid non-managed object pointer;
* @param p --- pointer on object to be freed
*/
// To redefine we need to ba accurate with std structures
// void operator delete (void * p) {
// TODO: needs check on managed; if true --- not to call free, otherwise call free
// TODO: then to call
	// if (!is_managed(p)) {
	// 	free(p);
	// }
// Problem: we don't know is object managed or not!
// because in managed case p points to the beggining of the object
// and p != chunck begin
// we can try to cast and IFF it is equal to the chunk begin
// then object is managed
// but it is too much to do for this simple perpouse
// }

/**
* @function gc_delete
* @detailed gc delete function
* @param chunk --- pointer on chunk to be freed
*/
void gc_delete (void * chunk) {
	dprintf("go.cpp: gc_delete\n");
	free(chunk);
}

extern void* __libc_stack_end;

void mark_stack(thread_handler* thread, bool full_gc) {
	void * stack_bottom = thread->stack_bottom;
	if (!stack_bottom) {
		stack_bottom = __libc_stack_end;
	}
	void** curr = (void**) thread->stack_top;
	assert(curr <= stack_bottom);
	while (curr <= stack_bottom) {
		if (is_heap_pointer(*curr)) {
			dprintf("possible heap pointer: %p\n", *curr);
			mark_dereferenced_root(*curr, full_gc);
		}
		curr++;
	}
}

void clean_deref_roots() {
	printf("Cleanup deref roots\n");
	thread_handler* handler = first_thread;
	while (handler) {
		while (!thread_in_safepoint(handler)) {
			dprintf("Waiting thread %d to reach safepoint\n", handler->thread);
			pthread_cond_wait(&safepoint_reached, &gc_mutex);
		}
		mark_stack(handler, false);
		handler = handler->next;
	}
	sweep_dereferenced_roots();
}

/**
* @function mark_and_sweep
* @detailed implements mark and sweep stop the world algorithm
*/
void mark_and_sweep() {
	printf("go.cpp: mark_and_sweep\n");
	dprintf("mark and sweep!\nbefore:");	//printDlMallocInfo(); fflush(stdout);
#ifdef DEBUGE_MODE
	live_object_count = 0;
	int i = 0;
	int over_count = 0;
#endif
	dprintf("roots: ");
	thread_handler* handler = first_thread;
	while (handler) {
		while (!thread_in_safepoint(handler)) {
			dprintf("Waiting thread %d to reach safepoint\n", handler->thread);
			pthread_cond_wait(&safepoint_reached, &gc_mutex);
		}
		// iterate root stack and call traversing function go
		mark_stack(handler, true);
		StackMap *stack_ptr = handler->stack;
		bool stack_overflow = false;
		for (Iterator root = stack_ptr->begin(); root <= stack_ptr->end(); root++) {/* walk through all roots*/
			stack_overflow |= go(get_next_obj(*root)); /* mark all available objects with mbit = 1*/
#ifdef DEBUGE_MODE
		i++;
	#endif
			dprintf("root %p ", get_next_obj(*root));
		}

#ifdef DEBUGE_MODE
	printf("\nroot count = %i; live_object_count = %zu\n", i, live_object_count);
#endif
		while (stack_overflow) {
			dprintf("mark_after_overflow\n");
			stack_overflow = mark_after_overflow();
#ifdef DEBUGE_MODE
		over_count++;
	#endif
		}
#ifdef DEBUGE_MODE
	printf("over_count = %i\n", over_count);
#endif
		handler = handler->next;
	}
	mark_fake_roots();
	// call sweep function (look at msmalloc)
	dprintf("call sweep\n");
	sweep();
	sweep_dereferenced_roots();
	dprintf("after: "); //printDlMallocInfo(); fflush(stdout);
}

void fix_roots() {
	thread_handler *handler = first_thread;
	while (handler) {
		StackMap *stack_ptr = handler->stack;
		for (Iterator root = stack_ptr->begin(); root <= stack_ptr->end(); root++) {/* walk through all roots*/
			fix_ptr(*root);
		}
		handler = handler->next;
	}
}