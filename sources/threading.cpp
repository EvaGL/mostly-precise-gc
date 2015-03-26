//
// Created by evagl on 26.03.15.
//
#include <sys/mman.h>
#include <assert.h>
#include <pthread.h>
#include "threading.h"
#include "stack.h"

pthread_mutex_t gc_mutex = PTHREAD_MUTEX_INITIALIZER;
thread_handler* gc_thread;
size_t threads = 1;

thread_handler* first_thread = nullptr;

void* start_routine(void* hand) {
     printf("Starting thread %d\n", pthread_self());
     thread_handler* handler = (thread_handler*) hand;
     void* arg = handler->stack_bottom;
     handler->stack_bottom = nullptr;
     handler->stack = StackMap::getInstance();
     handler->thread = pthread_self();
     pthread_mutex_lock(&gc_mutex);
          handler->next = first_thread;
          first_thread = handler;
          threads++;
     pthread_mutex_unlock(&gc_mutex);

     void* returnValue = handler->routine(arg);
     handler->routine = nullptr;
     printf("Finishing thread %d\n", pthread_self());
     return returnValue;
}

int thread_create(pthread_t *thread, const pthread_attr_t *attr, void* (*routine) (void*), void* arg) {
     if (!first_thread) {
          first_thread = (thread_handler*) mmap(0, sizeof(thread_handler),
                  PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
          assert(first_thread != MAP_FAILED);
          first_thread->thread = pthread_self();
          first_thread->next = nullptr;
          first_thread->stack = StackMap::getInstance();
     }
     thread_handler* handler = (thread_handler*) mmap(0, sizeof(thread_handler),
             PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
     assert(handler != MAP_FAILED);
     handler->routine = routine;
     handler->stack_bottom = arg;
     return pthread_create(thread, attr, start_routine, handler);
}