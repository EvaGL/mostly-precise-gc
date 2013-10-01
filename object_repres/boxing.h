# include <stdlib.h>
# include <limits.h>
# include <list>

using namespace std;

typedef unsigned long long word_t;

extern void * generic_box_simple		(void * ptr_to_st, size_t type);  

extern void * generic_box_unboxed_array 	(void * ptr_to_st, size_t len, size_t type);	
																														// 
extern void * generic_box_boxed_array		(list<void *> ptr_to_heap);    										

extern void * generic_box_struct		(void * ptr_to_heap, size_t len, list <size_t> num_el_inobj,list <size_t> offsets, list<size_t> type,list<size_t>mod, size_t all_size, size_t num_ptr);
