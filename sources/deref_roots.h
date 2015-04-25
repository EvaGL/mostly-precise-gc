#ifndef _DIPLOMA_DEREF_ROOTS_H_
#define _DIPLOMA_DEREF_ROOTS_H_

#include <stddef.h>
extern thread_local void* deref_roots;

void register_dereferenced_root(void*, size_t);
void mark_dereferenced_root(void*, void*);
void sweep_dereferenced_roots(void*);
#endif //_DIPLOMA_DEREF_ROOTS_H_
