#include <map>
#include "gc_new.h"
#include "../object_repres/go.h"
#include <cstdio>
#include <string>

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
std::vector <void *> offsets;
std::vector <void *> ptr_in_heap;
std::unordered_map <std::string, void *> list_meta_obj;
bool new_active = false;
size_t counter = 0;
