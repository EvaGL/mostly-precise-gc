#pragma once
#include <set>
#include <vector>
#include <utility>
#include <map>
#include <cassert>
#include "../gc_new/gc_new.h"

#include <cstdio>

struct ptr_list
{
	void *ptr;
	bool stack_ptr;

	ptr_list ()
	{
		next = 0;
		prev = 0;
		ptr = 0;
	}

	ptr_list (void *v)
	{
		next = 0;
		prev = 0;
		ptr = v;
	}


	ptr_list *next;
	ptr_list *prev;
};

ptr_list* add (ptr_list *&head, void *ptr);
void del (ptr_list *&head, ptr_list *me);
void clear (ptr_list *v);
void dec (ptr_list *me);
ptr_list* inc (void *p);
