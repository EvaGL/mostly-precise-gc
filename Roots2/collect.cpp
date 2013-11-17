/****************************************************************************************
	* File: collect.h
	* Description: This file consists code with realised functions from "collect.h"
*****************************************************************************************/
#include "collect.h"

StackMap stack_ptr; //!< stack of stack pointers

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

void inc (void *p)
{
	stack_ptr.inc(p);
}

void dec ()
{
	stack_ptr.dec();
}