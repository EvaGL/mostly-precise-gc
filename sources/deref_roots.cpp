#include "deref_roots.h"
#include "go.h"
#include <assert.h>
#include <stddef.h>
#include <msmalloc.h>
#include <stdio.h>

#define deref_mark(h) (h->p |= 1)
#define deref_unmark(h) (h->p &= ~1)
#define deref_is_marked(h) (h->p & 1)
#define plain_pointer(h) ((void*)(h->p & ~1))

typedef void* mspace;
extern "C" {
    void *create_mspace(size_t, int);
    void *mspace_malloc(mspace, size_t);
    void mspace_free(mspace, void *);
}
struct root_handler {
    size_t p;
    root_handler * next;
};
static mspace deref_space;
static constexpr size_t N = 17971;
root_handler* hashtable[N];

void register_dereferenced_root(void* root) {
    if (!deref_space) {
        deref_space = create_mspace(0, 0);
        assert(deref_space != nullptr);
        for (size_t i = 0; i < N; ++i) {
            hashtable[i] = nullptr;
        }
    }

    size_t hash = ((size_t)root >> 2) % N;
    root_handler* curr = hashtable[hash];
    while (curr) {
        if (plain_pointer(curr) == root) {
            return;
        }
        curr = curr->next;
    }
    root_handler * data = (root_handler *) mspace_malloc(deref_space, sizeof(root_handler));
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
            deref_mark(curr);
            go(p);
            return;
        }
        curr = curr->next;
    }
}

void sweep_dereferenced_roots() {
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
                mspace_free(deref_space, curr);
                curr = tmp;
            } else {
                deref_unmark(curr);
                prev = curr;
                curr = curr->next;
            }
        }
    }
}



