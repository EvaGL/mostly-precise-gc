//
// Created by evagl on 05.04.15.
//

#include "malloc.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <string.h>

struct block {
    size_t size;
    char data[0];
};

struct page {
    page* next;
    block* free_block;
    block first_block[0];
};

struct big_block {
    size_t size;
    big_block* next;
    char data[0];
};

static const size_t PAGE_SIZE = 16 * 4096;
static const size_t BIG_BLOCK_THRESHOLD = PAGE_SIZE - sizeof(block) - sizeof(page);
#define align(s) (s & 7 == 0 ? s : ((s >> 3) << 3))

#define PIN_FLAG 1
#define MARK_FLAG 2

#define block_size(b) (b->size & (~PIN_FLAG) & (~MARK_FLAG))

#define pin_block(b) (b->size |= PIN_FLAG)
#define unpin_block(b) (b->size &= ~PIN_FLAG)
#define block_is_pinned(b) (b->size & PIN_FLAG)

#define mark_block(b) (b->size |= MARK_FLAG)
#define unmark_block(b) (b->size &= ~MARK_FLAG)
#define block_is_marked(b) (b->size & MARK_FLAG)

#define forward_pointer(b) ((void*) b->data)
#define set_forward_pointer(b, ptr) ((void*)b->data = ptr)
#define next_block(b) ((block*)(((char*)b) + sizeof(block) + b->size))
#define page_is_full(p) ((char*)(p->free_block) == ((char*)p) + PAGE_SIZE)

page* first_page;
big_block* first_big_block;
pthread_mutex_t malloc_mutex = PTHREAD_MUTEX_INITIALIZER;

inline void* morecore(size_t size) {
    return mmap(nullptr, size, PROT_READ | PROT_WRITE,
            MAP_ANONYMOUS | MAP_SHARED, 0, 0);
}

inline page* request_new_page() {
    void* ptr = morecore(PAGE_SIZE);
    if (ptr == (void*) -1) {
        return nullptr;
    }
    page* new_page = (page*) ptr;
    new_page->free_block = new_page->first_block;
    new_page->free_block->size = BIG_BLOCK_THRESHOLD;
    new_page->next = nullptr;
    return new_page;
}

block* malloc_internal(size_t s, page** page_list) {
    if (s > BIG_BLOCK_THRESHOLD) {
        big_block* block = (big_block *) morecore(s + sizeof(big_block));
        if (block == MAP_FAILED) {
            return nullptr;
        }
        block->size = s;
        block->next = first_big_block;
        first_big_block = block;
        return block->data;
    }

    size_t s_and_block = s + sizeof(block);
    if (*page_list == nullptr) {
        if ((*page_list = request_new_page()) == nullptr) {
            return nullptr;
        }
    }
    page* curr_page = *page_list;
    while (page_is_full(curr_page) || block_size(curr_page->free_block) < s) {
        if (curr_page->next == nullptr) {
            curr_page->next = request_new_page();
            if (curr_page->next == nullptr) {
                return nullptr;
            }
        }
        curr_page = curr_page->next;
    }
    block* new_block = curr_page->free_block;
    curr_page->free_block = (block *) (((char*) new_block) + s_and_block);
    if (!page_is_full(curr_page)) {
        curr_page->free_block->size = new_block->size - s_and_block;
    }
    new_block->size = s;
    return new_block;
}

void* malloc(size_t s) {
    pthread_mutex_lock(&malloc_mutex);
    void* res = malloc_internal(align(s), &first_page)->data;
    pthread_mutex_unlock(&malloc_mutex);
    return res;
}

void sweep() {
    big_block* big = first_big_block;
    big_block* prev = nullptr;
    while (big != nullptr) {
        if (!block_is_marked(big) && !block_is_marked(big)) {
            big_block* next = big->next;
            if (prev == nullptr) {
                first_big_block = next;
            } else {
                prev->next = next;
            }
            munmap(big, sizeof(big) + block_size(big));
            big = next;
        } else {
            unpin_block(big);
            unmark_block(big);
            prev = big;
            big = big->next;
        }
    }

    page* alive_pages = nullptr;
    page* page = first_page;
    page* prev_page = nullptr;
    while (page != nullptr) {
        block* b = page->first_block;
        while (b != page->free_block) {
            if (block_is_pinned(b)) {
                if (prev_page == nullptr) {
                    first_page = page->next;
                } else {
                    prev_page->next = page->next;
                }
                page->next = alive_pages;
                alive_pages = page;
                break;
            }
            b = next_block(b);
        }
        if (b == page->first_block) {
            prev_page = page;
            page = page->next;
        }
    }

    page = first_page;
    while (page != nullptr) {
        block* b = page->first_block;
        while (b != page->free_block) {
            assert(!block_is_pinned(b));
            if (block_is_marked(b)) {
                size_t s = block_size(b);
                block* block_to_copy = malloc_internal(s, &alive_pages);
                assert(block_to_copy != nullptr); //TODO: THINK!!
                memcpy(block_to_copy->data, b->data, s);
                set_forward_pointer(b, block_to_copy->data);
            }
            b = next_block(b);
        }
        page = page->next;
    }
}