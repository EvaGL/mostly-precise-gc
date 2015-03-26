#include "deref_roots.h"
#include "go.h"
#include <assert.h>
#include <msmalloc.h>
#include <sys/mman.h>

#define deref_mark(h) (h->p |= 1)
#define deref_unmark(h) (h->p &= ~1)
#define deref_is_marked(h) (h->p & 1)
#define plain_pointer(h) ((void*)(h->p & ~1))
#define contains(h, p) (plain_pointer(h) <= p && p <= h->end)

struct root_handler {
    size_t p;
    void* end;
    root_handler * next;
};
static constexpr size_t N = 17971;
static thread_local root_handler *roots[N];
thread_local void* deref_roots = (void*) roots;
static thread_local bool init = false;
static thread_local root_handler* free_list = nullptr;

void register_dereferenced_root(void* root, size_t size) {
    if (!init) {
        for (size_t i = 0; i < N; ++i) {
            roots[i] = nullptr;
        }
        init = true;
    }

    size_t hash = ((size_t)root >> 2) % N;
    root_handler* curr = roots[hash];
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
    data->next = roots[hash];
    data->p = (size_t) root;
    data->end = (char*)root + size;
    roots[hash] = data;
}

void mark_dereferenced_root(void* root, void* h) {
    root_handler** hashtable = (root_handler**) h;
    size_t hash = ((size_t)root >> 2) % N;
    root_handler* curr = hashtable[hash];
    while (curr) {
        if (contains(curr, root)) {
            if (!deref_is_marked(curr)) {
                deref_mark(curr);
                go(plain_pointer(curr));
            }
            return;
        }
        curr = curr->next;
    }
}

void sweep_dereferenced_roots(void* h) {
    root_handler** hashtable = (root_handler**) h;
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



