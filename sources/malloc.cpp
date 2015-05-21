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
#include <unistd.h>

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

struct heap_part {
    void* start;
    void* end;
};

static heap_part parts[100];
static size_t parts_count = 0;

static size_t page_per_map = 32;
static size_t PAGE_SIZE = 0;
static size_t BIG_BLOCK_THRESHOLD;
static const float EXPAND_FACTOR = 2;
#define align(s) (s & 7 == 0 ? s : (((s >> 3) + 1) << 3))

#define PIN_FLAG 1
#define MARK_FLAG 2
#define DEAD_FLAG 4

#define block_size(b) (b->size & (~PIN_FLAG) & (~MARK_FLAG) & (~DEAD_FLAG))

#define pin_block(b) (b->size |= PIN_FLAG)
#define unpin_block(b) (b->size &= ~PIN_FLAG)
#define block_is_pinned(b) (b->size & PIN_FLAG)

#define mark_block(b) (b->size |= MARK_FLAG)
#define unmark_block(b) (b->size &= ~MARK_FLAG)
#define block_is_marked(b) (b->size & MARK_FLAG)

#define set_dead_flag(b) (b->size |= DEAD_FLAG)
#define block_is_dead(b) (b->size & DEAD_FLAG)

#define forward_pointer(b) (*((void**) b->data))
#define set_forward_pointer(b, ptr) (*((void**)b->data) = ptr)
#define next_block(b) ((block*)(((char*)b) + sizeof(block) + block_size(b)))
#define page_is_full(p) ((char*)(p->free_block) == ((char*)p) + PAGE_SIZE)

#define get_block(p) ((block*)((char*)p - sizeof(block)))

page* first_page = nullptr;
page* free_list = nullptr;
page* last_page = nullptr;
big_block* first_big_block = nullptr;
pthread_mutex_t malloc_mutex = PTHREAD_MUTEX_INITIALIZER;

long long nanotime( void ) {
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000000ll + ts.tv_nsec;
}

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
    for (int i = 0; i < parts_count; ++i) {
        if (parts[i].start <= ptr && ptr <= parts[i].end) {
            return true;
        }
    }
    for (big_block *bb = first_big_block; bb != nullptr; bb = bb->next) {
        if (bb <= ptr && ptr < bb->data + block_size(bb)) {
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

inline void* morecore(size_t size, bool is_big) {
    if (is_big) {
        return mmap(nullptr, size, PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    }
    void* result = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                                    MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    parts[parts_count].start = result;
    parts[parts_count].end = result + size;
    parts_count++;
    assert(result != MAP_FAILED);
    return result;
}

inline page* request_new_page(bool expand) {
    if (free_list == nullptr) {
        if (!expand) {
            return nullptr;
        }
        if (PAGE_SIZE == 0) {
            PAGE_SIZE = sysconf(_SC_PAGESIZE);
            BIG_BLOCK_THRESHOLD = PAGE_SIZE - sizeof(block) - sizeof(page);
        }
        void* ptr = morecore(page_per_map * PAGE_SIZE, false);
        if (ptr == (void*) -1) {
            return nullptr;
        }
        for (int i = 0; i < page_per_map; ++i) {
            page *new_page = (page *) ptr;
            new_page->next = free_list;
            free_list = new_page;
            ptr += PAGE_SIZE;
        }
        page_per_map *= EXPAND_FACTOR;
    }
    page* new_page = free_list;
    free_list = new_page->next;
    new_page->next = nullptr;
    new_page->free_block = new_page->first_block;
    new_page->free_block->size = BIG_BLOCK_THRESHOLD;
    return new_page;
}

inline block* malloc_in_page(size_t s, page* curr_page) {
    size_t s_and_block = s + sizeof(block);
    assert(block_size(curr_page->free_block) >= s);
    block* new_block = curr_page->free_block;
    curr_page->free_block = (block *) (((char*) new_block) + s_and_block);
    if (!page_is_full(curr_page)) {
        curr_page->free_block->size = new_block->size - s_and_block;
    }
    new_block->size = s;
    return new_block;
}

void* malloc_internal(size_t s, bool expand) {
    if (s > BIG_BLOCK_THRESHOLD) {
        big_block* b = (big_block *) morecore(s + sizeof(big_block), true);
        if (b == MAP_FAILED) {
            return nullptr;
        }
        b->size = s;
        b->next = first_big_block;
        first_big_block = b;
        return b->data;
    }

//    assert(last_page->next == nullptr);
    if (page_is_full(last_page) || block_size(last_page->free_block) < s) {
        last_page->next = request_new_page(expand);
        if (last_page->next == nullptr) {
            return nullptr;
        }
        last_page = last_page->next;
    }
    return malloc_in_page(s, last_page)->data;
}

void* gcmalloc(size_t s) {
    pthread_mutex_lock(&malloc_mutex);
    if (first_page == nullptr) {
        if ((first_page = request_new_page(true)) == nullptr) {
            return nullptr;
        }
        last_page = first_page;
    }
    void* res = malloc_internal(align(s), false);
    pthread_mutex_unlock(&malloc_mutex);
    if (res == nullptr) {
        printf("we need gc\n!");
        gc();
    }
    pthread_mutex_lock(&malloc_mutex);
    res = malloc_internal(align(s), true);
    pthread_mutex_unlock(&malloc_mutex);
    return res;
}

void fix_ptr(void *p) {
    if (p) {
        void *next = get_next_obj(p);
        if (next != nullptr) {
            void* data_begin = get_meta_inf(next);
            block *moved_block = get_block(data_begin);
            if (block_is_marked(moved_block)) {
                base_meta* moved_meta = (base_meta *) forward_pointer(moved_block);
                *(void **) p = move_ptr(*(void**)p, to_get_meta_inf(moved_meta));
            }
        }
    }
}

inline void copy_block(block* b, page* alive_pages) {
    size_t s = block_size(b);
    page* curr_page = alive_pages;
    while (page_is_full(curr_page) || block_size(curr_page->free_block) < s) {
        if (curr_page->next == nullptr) {
            curr_page->next = request_new_page(true);
            assert(curr_page->next != nullptr);
        }
        curr_page = curr_page->next;
    }
    block* block_to_copy = malloc_in_page(s, curr_page);
    memcpy(block_to_copy->data, b->data, s);
    set_forward_pointer(b, block_to_copy->data);
}

inline void sweep_big_blocks() {
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
}

void sweep() {
    long long start = nanotime();
    sweep_big_blocks();
    printf("sweep big blocks: %lld\n", nanotime() - start);
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
        } else { // block is pinned
            b = curr_page->first_block;
            while (b != curr_page->free_block) {
                if (!block_is_marked(b) && !block_is_pinned(b)) {
                    set_dead_flag(b);
                }
                unmark_block(b);
                unpin_block(b);
                b = next_block(b);
            }
        }
        curr_page = next_page;
    }
    printf("pin pages: %lld\n", nanotime() - start);

    curr_page = first_page;
    while (curr_page != nullptr) {
        block *b = curr_page->first_block;
        while (b != curr_page->free_block) {
            assert(!block_is_pinned(b));
            if (block_is_marked(b)) {
                if (alive_pages == nullptr) {
                    alive_pages = request_new_page(true);
                }
                copy_block(b, alive_pages);
            }
            b = next_block(b);
        }
        curr_page = curr_page->next;
    }
    printf("copy blocks: %lld\n", nanotime() - start);

    curr_page = alive_pages;
    while (curr_page != nullptr) {
        block *b = curr_page->first_block;
        while (b != curr_page->free_block) {
            assert(!block_is_pinned(b));
            assert(!block_is_marked(b));
            if (block_is_dead(b)) {
                b = next_block(b);
                continue;
            }
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
        if (curr_page->next == nullptr) {
            last_page = curr_page;
        }
        curr_page = curr_page->next;
    }

    fix_roots();
    printf("fix pointers: %lld\n", nanotime() - start);
    curr_page = first_page;
    while (curr_page != nullptr) {
        page* next = curr_page->next;
        curr_page->next = free_list;
        free_list = curr_page;
        curr_page = next;
    }
    first_page = alive_pages;
    printf("free pages: %lld\n", nanotime() - start);
}