//
// Created by evagl on 26.03.15.
//

#ifndef _DIPLOMA_THREADING_H_
#define _DIPLOMA_THREADING_H_
#include <pthread.h>
#include "stack.h"
#include "deref_roots.h"


struct thread_handler {
        pthread_t thread;
        void* arg;
        void* (*routine) (void*); // for init
        size_t flags;
        StackMap * stack;
        void* deref_roots;
        void* stack_top;
        void* stack_bottom;
        thread_handler* next;
};

#define thread_in_safepoint(h) (h->flags & 2)
#define enter_safepoint(h) (h->flags |= 2)
#define exit_safepoint(h) (h->flags |= ~2)

extern pthread_mutex_t gc_mutex;
extern pthread_cond_t gc_is_finished;
extern pthread_cond_t safepoint_reached;
extern thread_handler* first_thread;
extern thread_handler* gc_thread;

int thread_create(pthread_t *thread, const pthread_attr_t *attr,
        void* (*routine) (void*), void* arg);
void thread_join(pthread_t thread, void** thread_return);
void thread_exit(void** retval);
void thread_cancel(pthread_t thread);

thread_handler* get_thread_handler(pthread_t thread);
void wait_for_gc();
#endif //_DIPLOMA_THREADING_H_
