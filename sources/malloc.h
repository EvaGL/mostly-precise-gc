//
// Created by evagl on 05.04.15.
//

#ifndef _DIPLOMA_MALLOC_H_
#define _DIPLOMA_MALLOC_H_


#include <stddef.h>
/**
* @class base_meta
* @brief object meta base class
* @detailed realizes base object meta class (like interface for eatch object meta)
*/
class base_meta {
public:
    size_t *shell;	/**< pointer on the box(meta info struct for storing offsets) of object */
    size_t count;	/**< size of object */
    virtual void del_ptr () = 0;	/**< delete meta-ptr */
    virtual void* get_begin () = 0;	/**< get begin of object (pointer on meta)*/
};

    void *gcmalloc(size_t s);
    void fix_ptr(void *);
    void sweep();
    bool mark_after_overflow();
    size_t get_mark(void *);
    void mark(void *);
    void pin(void *);
    bool is_heap_pointer(void *);
#endif //_DIPLOMA_MALLOC_H_
