//
// Created by evagl on 05.04.15.
//

#ifndef _DIPLOMA_MALLOC_H_
#define _DIPLOMA_MALLOC_H_


#include <stddef.h>
#define stupid_malloc malloc
void* malloc(size_t s);
void fix_ptr(void*);
void sweep();
bool mark_after_overflow();
bool get_mark(void*);
void mark(void*);
#endif //_DIPLOMA_MALLOC_H_
