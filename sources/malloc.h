//
// Created by evagl on 05.04.15.
//

#ifndef _DIPLOMA_MALLOC_H_
#define _DIPLOMA_MALLOC_H_


#include <stddef.h>

    void *gcmalloc(size_t s);
    void fix_ptr(void *);
    void sweep();
    bool mark_after_overflow();
    size_t get_mark(void *);
    void mark(void *);
    void pin(void *);
    bool is_heap_pointer(void *);
#endif //_DIPLOMA_MALLOC_H_
