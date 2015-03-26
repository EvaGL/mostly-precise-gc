//
// Created by evagl on 26.03.15.
//
#include <sys/mman.h>
#include <assert.h>
#include <pthread.h>
#include "threading.h"

pthread_mutex_t gc_mutex = PTHREAD_MUTEX_INITIALIZER;
thread_handler* gc_thread;
size_t threads = 1;

thread_handler* first_thread = nullptr;

void remove_thread(pthread_t thread) {
     pthread_mutex_lock(&gc_mutex);
          thread_handler* curr = first_thread;
          thread_handler* prev = nullptr;
          while (!pthread_equal(curr->thread, thread)) {
               prev = curr;
               curr = curr->next;
          }
          if (prev == nullptr) {
               first_thread = curr->next;
          } else {
               prev->next = curr->next;
          }
     pthread_mutex_unlock(&gc_mutex);

     munmap(curr, sizeof(thread_handler));
}

void* start_routine(void* hand) {
     printf("Starting thread %d\n", pthread_self());
     thread_handler* handler = (thread_handler*) hand;
     handler->stack = StackMap::getInstance();
     handler->thread = pthread_self();
     handler->flags = 0;
     pthread_mutex_lock(&gc_mutex);
          handler->next = first_thread;
          first_thread = handler;
          threads++;
     pthread_mutex_unlock(&gc_mutex);

     void* returnValue = handler->routine(handler->arg);
     remove_thread(handler->thread);
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
     handler->arg = arg;
     return pthread_create(thread, attr, start_routine, handler);
}

void thread_join(pthread_t thread, void** thread_return) {
     pthread_t curr_thread = pthread_self();
     pthread_mutex_lock(&gc_mutex);
     thread_handler* curr = first_thread;
     while (!pthread_equal(curr->thread, curr_thread)) {
          curr = curr->next;
     }
     enter_safepoint(curr);
     pthread_mutex_unlock(&gc_mutex);

     pthread_join(thread, thread_return);

     pthread_mutex_lock(&gc_mutex);
     exit_safepoint(curr);
     pthread_mutex_unlock(&gc_mutex);
}

void thread_exit(void** retval) {
     remove_thread(pthread_self());
     pthread_exit(retval);
}

void thread_cancel(pthread_t thread) {
     remove_thread(thread);
     pthread_cancel(thread);
}