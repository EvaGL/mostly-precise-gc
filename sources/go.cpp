/****************************************************************************************
        * File: go.cpp
        * Description: realisation of functions from 'go.h'
        * Update: 22/10/13
*****************************************************************************************/

#include "taginfo.h"
#include "gc_new.h"
#include "stack.h"
#include "PointerList.h"

extern "C" {
	void mark(void*);
	size_t get_mark(void*);
	void sweep();
}

extern StackMap stack_ptr;
extern PointerList * offsets;

inline base_meta* get_meta_inf (void *v) {  /*!< get the block with meta_inf*/
//	if (DEBUGE_MODE) {
		printf("in get_meta_inf %p ", v);
		fflush(stdout);
//	}
	base_meta* res = (reinterpret_cast <base_meta*> (*(reinterpret_cast <size_t*> (reinterpret_cast <size_t>(v) - sizeof(base_meta*)))));
//	if (DEBUGE_MODE) {
		printf("end\n");
		fflush(stdout);
//	}
	//mark(res);
	return res;
}

inline void* get_next_obj(void *v) {  /* get the next object*/
	return reinterpret_cast <void*> (*((size_t *)v));
}

void go (void * v, bool mark_bit) {
	if (DEBUGE_MODE) {
		printf("\n\tin go ");
		fflush(stdout);
	}

	try {
		if (v == NULL) {
			return;
		}
		
		if (DEBUGE_MODE) {
			printf(" 1 ");
			fflush(stdout);
		}
		base_meta* bm = get_meta_inf(v); /* get metainformation from object*/

		if (get_mark(bm) != 0 || get_mark(v) != 0) { /* if marked --- return*/
			if (DEBUGE_MODE) {
				printf(" already marked \n ");
				fflush(stdout);
			}
			return;
		}
		if (DEBUGE_MODE) {
			printf(" 2 ");
			fflush(stdout);
		}
		void *shell = bm->shell;  /* saving ponter on meta object in shell */
		if (DEBUGE_MODE) {
			printf(" 3 ");
			fflush(stdout);
		}
		BLOCK_TAG* tag = (BLOCK_TAG *) shell; /* store shell in tag */
		if (DEBUGE_MODE) {
			printf(" 4: %p ", tag);
			fflush(stdout);
		}
		if (tag->model == 0) {  /* checking tag modell, correct if rang 1,2,4*/
			if (DEBUGE_MODE) {
				printf(" 4.1 ");
				fflush(stdout);
			}
			throw tag;
		}
		if (DEBUGE_MODE) {
			printf(" 5 ");
			fflush(stdout);
		}

		mark(bm);
		shell = (char *)shell + sizeof(BLOCK_TAG); /* get next part shell - offsets(num and offsets)*/	

		switch (tag->model) {
			case 1: {  /* if type of model == 1, it is complex object - struct*/
					for (size_t j = 0; j < bm->size; j++) { 
						void * this_offsets = shell;  /* get address of the offsets begin*/
						size_t n = *((size_t *)this_offsets);  /* count of offsets*/
						this_offsets = (char *)this_offsets + sizeof(size_t);  /* get first offset*/
						for (size_t i = 0; i < n; i++) {  /* walk throught offsets*/
							void *p = (char*)v + (*((POINTER_DESCR *)this_offsets)).offset;  /* get object by offset*/
							if (p) {
								go(get_next_obj(p), mark_bit);  /* go deeper and mark*/
							}
							this_offsets = (char *)this_offsets + sizeof(POINTER_DESCR);   /* get next pointer in this obj*/
						}
						v = (char *)v + tag->size;  /* get next object */
					}
				}
				break;
			case 2:  /* simple obj*/
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
	if (DEBUGE_MODE) {
		printf("in mark_and_sweep\n");
		fflush(stdout);
	}
	for(Iterator root = stack_ptr.begin(); root <= stack_ptr.end(); root++) {/* walk through all roots*/
		if (DEBUGE_MODE) {
			printf(" root %p; ", *root);
			fflush(stdout);
		}
		go (get_next_obj(*root), 1); /* mark all available objects with mbit = 1*/
	}
	if (DEBUGE_MODE) {
		printf("\nsweep\n");
		fflush(stdout);
	}
	sweep();
	if (DEBUGE_MODE) {
		printf("end sweep; end mark_and_sweep\n");
		fflush(stdout);
	}
}