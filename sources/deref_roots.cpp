#include "deref_roots.h"
#include "go.h"
#include <assert.h>
#include <stddef.h>
#include <msmalloc.h>
#include <stdio.h>
#include <sys/mman.h>

#define deref_mark(h) (h->p |= 1)
#define deref_unmark(h) (h->p &= ~1)
#define deref_is_marked(h) (h->p & 1)
#define plain_pointer(h) ((void*)(h->p & ~1))

struct root_handler {
    size_t p;
    root_handler * next;
};
static constexpr size_t N = 17971;
static root_handler* hashtable[N];
static bool init = false;
static root_handler* free_list = nullptr;

void register_dereferenced_root(void* root) {
    if (!init) {
        for (size_t i = 0; i < N; ++i) {
            hashtable[i] = nullptr;
        }
        init = true;
    }

    size_t hash = ((size_t)root >> 2) % N;
    root_handler* curr = hashtable[hash];
    while (curr) {
        if (plain_pointer(curr) == root) {
            return;
        }
        curr = curr->next;
    }
    if (free_list == nullptr) {
        void* space = mmap(nullptr, 4096 * sizeof(root_handler), PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        assert(space != MAP_FAILED);
        void* space_end = ((root_handler*) space) + 4096;
        for (root_handler* curr = (root_handler*) space; curr < space_end; ++curr) {
            curr->next = free_list;
            free_list = curr;
        }
    }
    root_handler * data = free_list;
    free_list = free_list->next;
    data->next = hashtable[hash];
    data->p = (size_t) root;
    hashtable[hash] = data;
}

void mark_dereferenced_root(void* root) {
    size_t hash = ((size_t)root >> 2) % N;
    root_handler* curr = hashtable[hash];
    while (curr) {
        void* p = plain_pointer(curr);
        if (p == root) {
            if (!deref_is_marked(curr)) {
                deref_mark(curr);
                go(p);
            }
            return;
        }
        curr = curr->next;
    }
}

void sweep_dereferenced_roots() {
    size_t count = 0;
    for (size_t i = 0; i < N; ++i) {
        root_handler *curr = hashtable[i];
        root_handler *prev = nullptr;
        while (curr) {
            if (!deref_is_marked(curr)) {
                root_handler *tmp = curr->next;
                if (!prev) {
                    hashtable[i] = curr->next;
                } else {
                    prev->next = curr->next;
                }
                curr->next = free_list;
                free_list = curr;
                curr = tmp;
                count++;
            } else {
                deref_unmark(curr);
                prev = curr;
                curr = curr->next;
            }
        }
    }
}



