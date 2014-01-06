/****************************************************************************************
        * File: go.cpp
        * Description: realisation of functions from 'go.h'
        * Update: 22/10/13
*****************************************************************************************/

#include "../Roots2/gc_ptr.h"
#include "taginfo.h"
#include "../gc_new/gc_new.h"
// #include "malloc.h"
//#include "mark.h"
#include <stdio.h>
//#include <cstdlib>
#include <vector>

extern "C" {
	void mark(void*);
	void unmark(void*);
	size_t get_mark(void*);
	void sweep();
}

//extern std::vector <void *> ptr_in_heap;

// extern std::vector <void *> offsets;
// extern std::unordered_map <std::string, void *> list_meta_obj;
extern MetaInformation * list_meta_obj;
extern StackMap stack_ptr;
extern PointerList * offsets;

void mark_meta() {
	mark(list_meta_obj);
	list_meta_obj->markMeta();
	offsets->markAll();
	mark(&stack_ptr);
	// mark(&list_meta_obj);
	// for (auto it = list_meta_obj.begin(); it != list_meta_obj.end(); ++it) {
	// 	mark(&it);
	// 	mark(it->second);
	// 	mark(&it->second);
	// }

	if (DEBUGE_MODE) {
		printf("meta list %p\n", &list_meta_obj);
	}
}

inline base_meta* get_meta_inf (void *v) {  /*!< get the block with meta_inf*/
	base_meta* res = (reinterpret_cast <base_meta*> (*(reinterpret_cast <size_t*> (reinterpret_cast <size_t>(v) - sizeof(base_meta*)))));
	mark(res);
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
			// printf("\n\tNULL pointer\n");
			// fflush(stdout);
			return;
		}
		
		if (DEBUGE_MODE) {
			printf(" 1 ");
			fflush(stdout);
		}
		base_meta* bm = get_meta_inf(v); /* get metainformation from object*/

		if (get_mark(bm) != 0) { /* if marked --- return*/
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
			printf(" 4:%p ", tag);
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

		// if (get_mark(v) != 0 || get_mark(bm) != 0) {
		// if (get_mark(bm) != 0) {
		// 	return;
		// }
		// bm->mbit = mark_bit;
		// mark(v);
		mark(bm);
		shell = (char *)shell + sizeof(BLOCK_TAG); /* get next part shell - offsets(num and offsets)*/	

		switch (tag->model) {
			case 1: {  /* if type of model == 1, it is complex object - struct*/
					for (size_t j = 0; j < bm->size; j++) { 
					// printf("\n\tfor 1 %zu\n", bm->size);
					// fflush(stdout); 
						void * this_offsets = shell;  /* get address of the offsets begin*/
						size_t n = *((size_t *)this_offsets);  /* count of offsets*/
						this_offsets = (char*)this_offsets + sizeof(size_t);  /* get first offset*/
						for (size_t i = 0; i < n; i++) {  /* walk throught offsets*/
					// printf("\n\tfor 2 %zu\n", n);
					// fflush(stdout); 
							void *p = (char*)v + (*((POINTER_DESCR *)this_offsets)).offset;  /* get object by offset*/
					// printf("1 ");
							if (p) {
					// printf("2 ");
								go(get_next_obj(p), mark_bit);  /* go deeper and mark*/
							}
					// printf("3 ");
							this_offsets = (char*)this_offsets + sizeof(POINTER_DESCR);   /* get next pointer in this obj*/
					// printf("4\n");
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
		printf("\n\tin mark_and_sweep\n");
		fflush(stdout);
	}
		mark_meta();
	if (DEBUGE_MODE) {
		printf("\n\tafter mark_meta();\n");
		fflush(stdout);
	}
	for(Iterator root = stack_ptr.begin(); root <= stack_ptr.end(); root++) {/* walk through all roots*/
		// printf("root %p\n", *root);
		go (get_next_obj(*root), 1); /* mark all available objects with mbit = 1*/
		// printf("end root\n");
	}
	if (DEBUGE_MODE) {
		printf("\n\tafter roots iterate\n");
		fflush(stdout);

		printf("\n\tsweep\n");
		fflush(stdout);
	}
		sweep();
	if (DEBUGE_MODE) {
		printf("\n\tend sweep\n");
		fflush(stdout);
	}
}

// void go (void *v, bool mark_bit) {  /* walk and mark throught the objects which we can get by walking from roots*/
// 	try {
// 		if (v == 0) { /* if the pointer on object null just return*/
// 			return;
// 		}

// 		base_meta* bm = get_meta_inf(v); /* get metainformation from object*/
// 		void *shell = bm->shell;  /* saving ponter on meta object in shell */
// 		BLOCK_TAG* tag = (BLOCK_TAG *) shell; /* store shell in tag */

// 		if (tag->model == 0) {  /* checking tag modell, correct if rang 1,2,4*/
// 			throw tag;
// 		}
// 		if (bm->mbit == mark_bit) {  /* if it was marked/unmarked before - done*/
// 			return;
// 		}

// 		bm->mbit = mark_bit;  /* change if it was not marked*/
// 		shell = (char *)shell + sizeof(BLOCK_TAG); /* get next part shell - offsets(num and offsets)*/	

// 		switch (tag->model) {
// 			case 1: {  /* if type of model == 1, it is complex object - struct*/
// 					for (size_t j = 0; j < bm->size; j++) {  
// 						void *offsets = shell;  /* get address of the offsets begin*/
// 						size_t n = *((size_t *)offsets);  /* count of offsets*/
// 						offsets = (char*)offsets + sizeof(size_t);  /* get first offset*/
// 						for (size_t i = 0; i < n; i++) {  /* walk throught offsets*/
// 							void *p = (char*)v + (*((POINTER_DESCR *)offsets)).offset;  /* get object by offset*/
// 							go(get_next_obj(p), mark_bit);  /* go deeper and mark*/
// 							offsets = (char*)offsets + sizeof(POINTER_DESCR);   /* get next pointer in this obj*/
// 						}
// 						v = (char *)v + tag->size;  /* get next object */
// 					}
// 				}
// 				break;
// 			case 2:  /* simple obj*/
// 				break;
// 			case 4:  /* unboxed_array*/
// 				break;
// 			default:
// 				throw tag;	
// 				break;
// 		}
// 	} catch(BLOCK_TAG* tag) {
// 		printf("Error of data representation!\nAttention, null - value of tag-model\n");
// 		printf("tag value:%p\n", tag);
// 		printf("tag-model value: %u\n", tag->model);	
// 		printf("tag-size value: %lu\n", tag->size);	
// 	} catch(...) {
// 		printf("UNEXPECTED ERROR!!! CHECK tag->mbit");
// 	}
// }

// void mark_and_sweep () {
// 	struct mallinfo mi;
// 	mi = mallinfo();
// 	printf ("total allocated space before m&s %i\n", mi.uordblks);

// 	for(Iterator root = stack_ptr.begin(); root <= stack_ptr.end(); root++) {/* walk through all roots*/
// 		go (get_next_obj(*root), 1); /* mark all available objects with mbit = 1*/
// 	}


// 	std::vector <void*> to_erase;  /* garbage list*/
// 	std::vector <void*> new_ptr_in_heap;  /* live objects list*/

// 	for (size_t i = 0; i < ptr_in_heap.size(); i++) {  /* walk in heap*/	
// 		base_meta* m_inf = get_meta_inf(ptr_in_heap[i]) ;  /* get information about mbit of object*/
// 		if (m_inf->mbit == 1) {
// 			new_ptr_in_heap.push_back(ptr_in_heap[i]);  /* if alive - live*/
// 		} else { 
// 			to_erase.push_back(ptr_in_heap[i]); /* if dead - be killed*/
// 		}
// 	}
// 	for (size_t i = 0; i < to_erase.size(); i++) {  /* clean all connected with dead object information*/
// 		base_meta* m_inf = get_meta_inf(to_erase[i]);
// 		m_inf->del_ptr();
// 		free (m_inf->get_begin());
// 	}

// 	ptr_in_heap.clear();  
// 	ptr_in_heap = new_ptr_in_heap;

// 	for (size_t i = 0; i < ptr_in_heap.size(); i++) {  /* umark all alive objects in heap*/
// 		base_meta* m_inf = get_meta_inf(ptr_in_heap[i]);
// 		m_inf->mbit = 0;
// 	}

// 	mi = mallinfo();
// 	printf ("total allocated space after m&s %i\n", mi.uordblks);
// }