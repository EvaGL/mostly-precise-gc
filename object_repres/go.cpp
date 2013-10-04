#include "../Roots2/gc_ptr.h"
#include "taginfo.h"
#include "../gc_new/gc_new.h"
#include "malloc.h"
#include <cstdlib>
#include <vector>

extern std::vector <void *> ptr_in_heap;
extern ptr_list* all_ptr;

inline base_meta* get_meta_inf (void *v) {
	return (reinterpret_cast <base_meta*> (*(reinterpret_cast <size_t*> (reinterpret_cast <size_t>(v) - sizeof(base_meta*)))));
}

inline void* get_next_obj(void *v) {
	return reinterpret_cast <void*> (*((size_t *)v));
}

void go (void *v, bool mark_bit) {
	try {
		if (v == 0) {
			return;
		}

		base_meta* bm = get_meta_inf(v);
		void *shell = bm->shell;
		BLOCK_TAG* tag = (BLOCK_TAG *) shell;

		if (tag->model == 0) {
			throw tag;
		}
		if (bm->mbit == mark_bit) {
			return;
		}

		bm->mbit = mark_bit;
		shell = shell + sizeof(BLOCK_TAG);		

		switch (tag->model) {
			case 1: {
					for (size_t j = 0; j < bm->size; j++) {
						void *offsets = shell;
						size_t n = *((size_t *)offsets);
						offsets += sizeof(size_t);
						for (size_t i = 0; i < n; i++) {
							void *p = v + (*((POINTER_DESCR *)offsets)).offset;
							go(get_next_obj(p), mark_bit);
							offsets += sizeof(POINTER_DESCR); 
						}
						v = v + tag->size;
					}
				}
				break;
			case 2:
				break;
			case 4:
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

	for (ptr_list* root = all_ptr; root != 0; root = root->next) {
		go (get_next_obj(root->ptr), 1);
	}

	std::vector <void*> to_erase;
	std::vector <void*> new_ptr_in_heap;

	for (size_t i = 0; i < ptr_in_heap.size(); i++) {	
		base_meta* m_inf = get_meta_inf(ptr_in_heap[i]) ;
		if (m_inf->mbit == 1) {
			new_ptr_in_heap.push_back(ptr_in_heap[i]);
		} else {
			to_erase.push_back(ptr_in_heap[i]);
		}
	}
	for (size_t i = 0; i < to_erase.size(); i++) {
		base_meta* m_inf = get_meta_inf(to_erase[i]);
		m_inf->del_ptr();
		free (m_inf->get_begin());
	}

	ptr_in_heap.clear();
	ptr_in_heap = new_ptr_in_heap;

	for (size_t i = 0; i < ptr_in_heap.size(); i++) {
		base_meta* m_inf = get_meta_inf(ptr_in_heap[i]);
		m_inf->mbit = 0;
	}

	mi = mallinfo();
	printf ("total allocated space after m&s %i\n", mi.uordblks);
}