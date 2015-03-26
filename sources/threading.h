//
// Created by evagl on 26.03.15.
//

#ifndef _DIPLOMA_THREADING_H_
#define _DIPLOMA_THREADING_H_
#include <pthread.h>
#include "stack.h"

struct thread_handler {
        pthread_t thread;
        void* stack_bottom;
        void* (*routine) (void*); // for init
        StackMap * stack;
        thread_handler* next;
};

extern pthread_mutex_t gc_mutex;
extern thread_handler* first_thread;
extern thread_handler* gc_thread;

int thread_create(pthread_t *thread, const pthread_attr_t *attr,
        void* (*routine) (void*), void* arg);
#endif //_DIPLOMA_THREADING_H_
