//
// Created by evagl on 05.04.15.
//

#include "malloc.h"
#include "taginfo.h"
#include "go.h"
#include <sys/mman.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

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
    big_block* next;
    size_t size;
    char data[0];
};

static const size_t PAGE_SIZE = 16 * 4096;
static const size_t BIG_BLOCK_THRESHOLD = PAGE_SIZE - sizeof(block) - sizeof(page);
#define align(s) (s & 7 == 0 ? s : (((s >> 3) + 1) << 3))

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
#define set_forward_pointer(b, ptr) (((void**)b->data)[0] = ptr)
#define next_block(b) ((block*)(((char*)b) + sizeof(block) + block_size(b)))
#define page_is_full(p) ((char*)(p->free_block) == ((char*)p) + PAGE_SIZE)

#define get_block(p) ((block*)((char*)p - sizeof(block)))
#define get_block_from_big_block(bb) (get_block(bb->data))

page* first_page;
big_block* first_big_block;
pthread_mutex_t malloc_mutex = PTHREAD_MUTEX_INITIALIZER;

size_t get_mark(void* ptr) {
    return block_is_marked(get_block(ptr));
}

void mark(void* ptr) {
    mark_block(get_block(ptr));
}

void pin(void* ptr) {
    pin_block(get_block(ptr));
}

bool is_heap_pointer(void* ptr) {
    for(big_block* b = first_big_block; b != nullptr; b = b->next) {
        if (ptr >= b && (char*)b + sizeof(big_block) + b->size > ptr) {
            return true;
        }
    }

    for (page* p = first_page; p != nullptr; p = p->next) {
        if (ptr >= p && (char*)p + PAGE_SIZE > ptr) {
            return true;
        }
    }
    return false;
}

bool mark_after_overflow() {
    bool is_overflow = false;
    for (big_block* b= first_big_block; b != nullptr; b = b->next) {
        if(block_is_marked(b) && go(&b->data)) {
            is_overflow = true;
        }
    }
    for (page* p = first_page; p != nullptr; p = p->next) {
        block* b = p->first_block;
        while (b != p->free_block) {
            if(block_is_marked(b) && go(&b->data)) {
                is_overflow = true;
            }
            b = next_block(b);
        }
    }
    return is_overflow;
}

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

class base_meta {
public:
    void *shell;	/**< pointer on the box(meta info struct for storing offsets) of object */
    void * ptrptr;	/**< pointer to the real object begin */
    size_t size;	/**< size of object */
    virtual void del_ptr () = 0;	/**< delete meta-ptr */
    virtual void* get_begin () = 0;	/**< get begin of object (pointer on meta)*/
};


block* malloc_internal(size_t s, page** page_list) {
    if (s > BIG_BLOCK_THRESHOLD) {
        big_block* b = (big_block *) morecore(s + sizeof(big_block));
        if (b == MAP_FAILED) {
            return nullptr;
        }
        b->size = s;
        b->next = first_big_block;
        first_big_block = b;
        return get_block_from_big_block(b);
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

void* gcmalloc(size_t s) {
    pthread_mutex_lock(&malloc_mutex);
    void* res = malloc_internal(align(s), &first_page)->data;
    pthread_mutex_unlock(&malloc_mutex);
    return res;
}

void fix_ptr(void *p) {
    if (p) {
        void *next = get_next_obj(p);
        if (next != nullptr) {
            block *moved_block = get_block(next);
            if (block_is_marked(moved_block)) {
                //*(void **) p = move_ptr(p, forward_pointer(moved_block)); FIXME: !!!
            }
        }
    }
}

void sweep() {
    big_block *big = first_big_block;
    big_block *prev = nullptr;
    while (big != nullptr) {
        if (!block_is_marked(big) && !block_is_pinned(big)) {
            big_block *next = big->next;
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

    page *alive_pages = nullptr;
    page *curr_page = first_page;
    page *prev_page = nullptr;
    while (curr_page != nullptr) {
        block *b = curr_page->first_block;
        page* next_page = curr_page->next;
        while (b != curr_page->free_block) {
            if (block_is_pinned(b)) {
                if (prev_page == nullptr) {
                    first_page = curr_page->next;
                } else {
                    prev_page->next = curr_page->next;
                }
                curr_page->next = alive_pages;
                alive_pages = curr_page;
                break;
            }
            b = next_block(b);
        }
        if (b == curr_page->free_block) {
            prev_page = curr_page;
        }
        curr_page = next_page;
    }

    curr_page = first_page;
    while (curr_page != nullptr) {
        block *b = curr_page->first_block;
        while (b != curr_page->free_block) {
            assert(!block_is_pinned(b));
            if (block_is_marked(b)) {
                size_t s = block_size(b);
                block *block_to_copy = malloc_internal(s, &alive_pages);
                assert(block_to_copy != nullptr); //TODO: THINK!!
                memcpy(block_to_copy->data, b->data, s);
                set_forward_pointer(b, block_to_copy->data);
            }
            b = next_block(b);
        }
        curr_page = curr_page->next;
    }

    curr_page = alive_pages;
    while (curr_page != nullptr) {
        block *b = curr_page->first_block;
        while (b != curr_page->free_block) {
            unmark_block(b);
            unpin_block(b);
            void *data = b->data + sizeof(base_meta);
            base_meta *meta = (base_meta *) b->data;
            BLOCK_TAG *tag = (BLOCK_TAG *) meta->shell;
            size_t sizeType = 0;
            size_t sizeArray = 0;
            size_t n = 0;
            char* offsets_base = nullptr;
            switch (tag->model) {
                case 1:
                    sizeArray = 1;
                    offsets_base = (char *) tag;
                    break;
                case 3:
                    sizeType = tag->num_of_el;
                    sizeArray = tag->size;
                    offsets_base = (char *) tag->ptr;
                    break;
                default:
                    break;
            }

            if (offsets_base != nullptr) {
                n = *(size_t *) (offsets_base + sizeof(BLOCK_TAG));
            }
            for (size_t t = 0; t < sizeArray; t++, data = (void *) ((char *) data + sizeType)) {
                void *this_offsets = offsets_base + sizeof(BLOCK_TAG) + sizeof(size_t);
                for (size_t i = 0; i < n; i++) {
                    fix_ptr((char *) data + (*((POINTER_DESCR *) this_offsets)).offset);
                    this_offsets = (char *) this_offsets + sizeof(POINTER_DESCR);
                }
            }
            b = next_block(b);
        }
        curr_page = curr_page->next;
    }

    fix_roots();
    curr_page = first_page;
    while (curr_page != nullptr) {
        page* next = curr_page->next;
        munmap(curr_page, PAGE_SIZE);
        curr_page = next;
    }
    first_page = alive_pages;
}