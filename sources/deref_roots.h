#ifndef _DIPLOMA_DEREF_ROOTS_H_
#define _DIPLOMA_DEREF_ROOTS_H_

void register_dereferenced_root(void*);
void mark_dereferenced_root(void*);
void sweep_dereferenced_roots();
#endif //_DIPLOMA_DEREF_ROOTS_H_
