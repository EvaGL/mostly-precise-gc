#include "collect.h"

ptr_list *all_ptr = 0;

ptr_list* add (ptr_list *&head, void *ptr)
{
	ptr_list *nw = new ptr_list(ptr);
	nw->next = head;
	if (head != 0) {
		head->prev = nw;
	}
	return head = nw;
}

void del (ptr_list *&head, ptr_list *me)
{
	if (me == head) {
		head = me->next;
	}
	if (me->prev) {
		(me->prev)->next = me->next;
	}
	if (me->next) {
		(me->next)->prev = me->prev;
	}
	delete me;
}


void clear (ptr_list *v)
{
	if (v->next != 0) {
		clear(v->next);
	}
	delete v;
}

void dec (ptr_list *me)
{
	del(all_ptr, me);
}

ptr_list* inc (void *p)
{
	return add(all_ptr, p);
}

