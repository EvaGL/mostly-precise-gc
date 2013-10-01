
#include "boxing2.h"
#include <iterator>
#include "taginfo.h"
#include <cstdio>



void * generic_box_simple () { 
	return create_generic_object(0, 0, 1); 
}

void * generic_box_unboxed_array(size_t len) {
	return create_unboxed_array(len);
}

void * generic_box_boxed_array (size_t len)  { 
	return create_boxed_array(len);
}

void * generic_box_struct (std::list <size_t> offsets_ptr, size_t size, size_t num_el) {
	void* object;
	try { 
		object = create_generic_object(offsets_ptr.size(), size, num_el);
		std::list <size_t>::iterator it_offset = offsets_ptr.begin();
		POINTER_DESCR descr;
		for ( size_t iter_p = 1; iter_p < offsets_ptr.size() + 1; iter_p++, it_offset++) {
					descr = {*it_offset, 0};
					set_ptr_descr(object, iter_p, descr);  
		}
	}
	catch (...) {
		printf("Error! Couldn't create generic box struct!");
	}
	return object;	
}


		


 

