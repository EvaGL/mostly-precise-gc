#include "deref_roots.h"
#include <assert.h>
#include <stddef.h>
#include <msmalloc.h>

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
struct deref_root_handler {
    size_t p;
    deref_root_handler* next;
};
static mspace deref_space;
deref_root_handler* first;

void register_dereferenced_root(void* root) {
    if (!deref_space) {
        deref_space = create_mspace(0, 0);
        assert(deref_space != nullptr);
    }
    deref_root_handler* data = (deref_root_handler*) mspace_malloc(deref_space, sizeof(deref_root_handler));
    data->next = first;
    data->p = (size_t) root;
    first = data;
}

void mark_dereferenced_root(void* root) {
    deref_root_handler* curr = first;
    while (curr) {
        void* p = plain_pointer(curr);
        if (p == root) {
            deref_mark(curr);
            mark(p);
            return;
        }
        curr = curr->next;
    }
}

void sweep_dereferenced_roots() {
    deref_root_handler* curr = first;
    deref_root_handler* prev = nullptr;
    while (curr) {
        if (!deref_is_marked(curr)) {
            deref_root_handler* tmp = curr->next;
            if (!prev) {
                first = curr->next;
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



