//
// Created by evagl on 26.03.15.
//
#include <sys/mman.h>
#include <assert.h>
#include <pthread.h>
#include "threading.h"

pthread_mutex_t gc_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t gc_finished_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gc_is_finished = PTHREAD_COND_INITIALIZER;
pthread_cond_t safepoint_reached = PTHREAD_COND_INITIALIZER;
thread_handler* gc_thread;

thread_handler* first_thread = nullptr;

void remove_thread(pthread_t thread) {
     without_gc_before()
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
     without_gc_after()
     munmap(curr, sizeof(thread_handler));
}

void* start_routine(void* hand) {
     thread_handler* handler = (thread_handler*) hand;
     handler->stack = StackMap::getInstance();
     handler->deref_roots = deref_roots;
     handler->thread = pthread_self();
     handler->stack_bottom = __builtin_frame_address(0);
     handler->flags = 0;
     handler->tlflags = &new_obj_flags_tl_instance;
     dprintf("Starting thread %d\n", handler->thread);

     pthread_mutex_lock(&gc_mutex);
     if (gc_thread) {
         pthread_mutex_unlock(&gc_mutex);
         dprintf("waiting for gc before starting thread %d\n", handler->thread);
         pthread_cond_wait(&gc_is_finished, &gc_mutex);
     }
     handler->next = first_thread;
     first_thread = handler;
     pthread_mutex_unlock(&gc_mutex);

     void* returnValue = handler->routine(handler->arg);
     dprintf("Finishing thread %d\n", handler->thread);
     remove_thread(handler->thread);
     return returnValue;
}

void create_first_handler() {
     first_thread = (thread_handler*) mmap(0, sizeof(thread_handler),
             PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
     assert(first_thread != MAP_FAILED);
     first_thread->thread = pthread_self();
     first_thread->next = nullptr;
     first_thread->stack = StackMap::getInstance();
     first_thread->stack_bottom = nullptr;
     first_thread->deref_roots = deref_roots;
     first_thread->flags = 0;
     first_thread->tlflags = &new_obj_flags_tl_instance;
}

int thread_create(pthread_t *thread, const pthread_attr_t *attr, void* (*routine) (void*), void* arg) {
     if (!first_thread) {
          create_first_handler();
     }
     thread_handler* handler = (thread_handler*) mmap(0, sizeof(thread_handler),
             PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
     assert(handler != MAP_FAILED);
     handler->routine = routine;
     handler->arg = arg;
     return pthread_create(thread, attr, start_routine, handler);
}

void thread_join(pthread_t thread, void** thread_return) {
     pthread_mutex_lock(&gc_mutex);
     thread_handler* curr = get_thread_handler(pthread_self());
     curr->stack_top = __builtin_frame_address(0);
     enter_safepoint(curr);
     if (gc_thread) {
          dprintf("Thread %d reached safepoint in join\n", curr->thread);
          pthread_cond_signal(&safepoint_reached);
     }
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

thread_handler* get_thread_handler(pthread_t thread) {
     if (!first_thread) {
          create_first_handler();
     }
     thread_handler* curr = first_thread;
     assert(curr != nullptr);
     while (!pthread_equal(curr->thread, thread)) {
          curr = curr->next;
     }
     return curr;
}
