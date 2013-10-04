# pragma once
# include <stdlib.h>
# include <limits.h>
# include <list>

typedef unsigned long long word_t;

extern void * generic_box_simple		();
extern void * generic_box_unboxed_array (size_t len);
extern void * generic_box_boxed_array	(size_t len);
extern void * generic_box_struct		(std::list <size_t> offsets_ptr, size_t size, size_t num_el);