/****************************************************************************************
        * File: go.cpp
        * Description: realisation of functions from 'go.h'
        * Update: 31/03/14
*****************************************************************************************/

#include "taginfo.h"
#include "gc_new.h"
#include "stack.h"
#include "PointerList.h"
#include "fake_roots.h"

// #define DEBUGE_MODE

extern StackMap stack_ptr;
extern PointerList * offsets;

inline void* get_next_obj(void *v) {  /* get the next object*/
	return reinterpret_cast <void*> (*((size_t *)v));
}

inline base_meta* get_meta_inf (void *v) {  /*!< get the block with meta_inf*/
	base_meta* res = (reinterpret_cast <base_meta*> (*(reinterpret_cast <size_t*> (reinterpret_cast <size_t>(v) - sizeof(base_meta*)))));
	return res;
}

void go (void * v) {
	try {
		if (v == NULL || !is_heap_pointer(v)) {
			#ifdef DEBUGE_MODE
				printf(" %p !is_heap_pointer \n ", v);
			#endif
			return;
		}
		#ifdef DEBUGE_MODE
			printf("\nin go\n");
			fflush(stdout);
		#endif
		
		base_meta* bm = get_meta_inf(v); /* get metainformation from object*/

		if (get_mark(bm) != 0 || get_mark(v) != 0) { /* if marked --- return*/
			#ifdef DEBUGE_MODE
				printf(" already marked \n ");
				fflush(stdout);
			#endif
			return;
		}
		void *shell = bm->shell;  /* saving ponter on meta object in shell */
		BLOCK_TAG* tag = (BLOCK_TAG *) shell; /* store shell in tag */
		if (tag->model == 0) {  /* checking tag modell, correct if rang 1,2,4*/
			throw tag;
		}

		mark(bm);
		#ifdef DEBUGE_MODE
			printf("tag->model:%i \n", tag->model);
			fflush(stdout);
		#endif

		switch (tag->model) {
			case 1: {  /* boxed object */
					size_t n = *(size_t *)((char *)shell + sizeof(BLOCK_TAG));  /* count of offsets*/
					void * this_offsets = (char *)shell + sizeof(BLOCK_TAG) + sizeof(size_t);  /* get first offset*/
					for (size_t i = 0; i < n; i++) {  /* walk throught offsets*/
						void *p = (char*)v + (*((POINTER_DESCR *)this_offsets)).offset;  /* get object by offset*/
						if (p) {
							go(get_next_obj(p));  /* go deeper and mark*/
						}
						this_offsets = (char *)this_offsets + sizeof(POINTER_DESCR);   /* get next pointer in this obj*/
					}
					v = (char *)v + tag->size;  /* get next object */
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
								go(get_next_obj(p));
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
	} catch(...) {
		printf("UNEXPECTED ERROR!!! CHECK tag->mbit");
		fflush(stdout);
	}
}

void mark_and_sweep () {
	printf("before m&s "); printDlMallocInfo();
	mark_fake_roots();
	for(Iterator root = stack_ptr.begin(); root <= stack_ptr.end(); root++) {/* walk through all roots*/
		go (get_next_obj(*root)); /* mark all available objects with mbit = 1*/
	}
	sweep();
	printf("after m&s "); printDlMallocInfo();
}