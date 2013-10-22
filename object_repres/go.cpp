/****************************************************************************************
        * File: go.cpp
        * Description: realisation of functions from 'go.h'
        * Update: 22/10/13
*****************************************************************************************/

#include "../Roots2/gc_ptr.h"
#include "taginfo.h"
#include "../gc_new/gc_new.h"
#include "malloc.h"
#include <cstdlib>
#include <vector>

extern std::vector <void *> ptr_in_heap; /* list of pointers in heap*/

extern ptr_list* all_ptr; /* all created pointers*/

inline base_meta* get_meta_inf (void *v) {  /* get the block with meta_inf*/
	return (reinterpret_cast <base_meta*> (*(reinterpret_cast <size_t*> (reinterpret_cast <size_t>(v) - sizeof(base_meta*)))));
}

inline void* get_next_obj(void *v) {  /* get the next object*/
	return reinterpret_cast <void*> (*((size_t *)v));
}

void go (void *v, bool mark_bit) {  /* walk and mark throught the objects which we can get by walking from roots*/
	try {
		if (v == 0) { /* if the pointer on object null just return*/
			return;
		}

		base_meta* bm = get_meta_inf(v); /* get metainformation from object*/
		void *shell = bm->shell;  /* saving tag info in shell */
		BLOCK_TAG* tag = (BLOCK_TAG *) shell; /* resave shell in tag */

		if (tag->model == 0) {  /* checking tag modell, correct if rang 1,2,4*/
			throw tag;
		}
		if (bm->mbit == mark_bit) {  /* if it was marked/unmarked before - done*/
			return;
		}

		bm->mbit = mark_bit;  /* change if it was not marked*/
		shell = shell + sizeof(BLOCK_TAG); /* get next part shell - offsets(num and offsets)*/	

		switch (tag->model) {
			case 1: {  /* if type of model == 1, it is complex object - struct*/
					for (size_t j = 0; j < bm->size; j++) {  
						void *offsets = shell;  /* get address of the offsets begin*/
						size_t n = *((size_t *)offsets);  /* count of offsets*/
						offsets += sizeof(size_t);  /* get first offset*/
						for (size_t i = 0; i < n; i++) {  /* walk throught offsets*/
							void *p = v + (*((POINTER_DESCR *)offsets)).offset;  /* get object by offset*/
							go(get_next_obj(p), mark_bit);  /* mark, if we need it*/
							offsets += sizeof(POINTER_DESCR);   /* add new offset*/
						}
						v = v + tag->size;  /* set pointer om object*/
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
	} catch(...) {
		printf("UNEXPECTED ERROR!!! CHECK tag->mbit");
	}
}

void mark_and_sweep () {
	struct mallinfo mi;
	mi = mallinfo();
	printf ("total allocated space before m&s %i\n", mi.uordblks);

	for (ptr_list* root = all_ptr; root != 0; root = root->next) {  /* walk through all pointers in stack*/
		go (get_next_obj(root->ptr), 1);  /* mark all roots with mbit = 1*/ 
	}

	std::vector <void*> to_erase;  /* garbage list*/
	std::vector <void*> new_ptr_in_heap;  /* live objects list*/

	for (size_t i = 0; i < ptr_in_heap.size(); i++) {  /* walk in heap*/	
		base_meta* m_inf = get_meta_inf(ptr_in_heap[i]) ;  /* get information about mbit of object*/
		if (m_inf->mbit == 1) {
			new_ptr_in_heap.push_back(ptr_in_heap[i]);  /* if alive - live*/
		} else { 
			to_erase.push_back(ptr_in_heap[i]); /* if dead - be killed*/
		}
	}
	for (size_t i = 0; i < to_erase.size(); i++) {  /* clean all connected with dead object information*/
		base_meta* m_inf = get_meta_inf(to_erase[i]);
		m_inf->del_ptr();
		free (m_inf->get_begin());
	}

	ptr_in_heap.clear();  
	ptr_in_heap = new_ptr_in_heap;

	for (size_t i = 0; i < ptr_in_heap.size(); i++) {  /* umark all alive objects in heap*/
		base_meta* m_inf = get_meta_inf(ptr_in_heap[i]);
		m_inf->mbit = 0;
	}

	mi = mallinfo();
	printf ("total allocated space after m&s %i\n", mi.uordblks);
}
