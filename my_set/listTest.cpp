#include <cstdio>
#include <cstdlib>
#include <cassert>
#include "gc.h"

struct list
{
	int val;
	gc_ptr <list> next;

	list ()
	{
		val = 0;
		next = 0;
	}

	list (int v)
	{
		val = v;
		next = 0;
	}
};

void add (gc_ptr<list> &head, gc_ptr<list> elem)
{
	elem->next = head;
	head = elem;
}

void test (int STEP_COUNT, int ELEM_COUNT)
{
	for (int i = 0; i < STEP_COUNT; i++)
	{
		gc_ptr<list> head = 0;
		for (int j = 0; j < ELEM_COUNT; j++)
			add(head, gc_new <list> (1));
	}
}

int main ()// (int argc, char *argv[])
{
//	assert(argc == 3);
	int STEP_COUNT = 1000, ELEM_COUNT = 10000;

	//sscanf (argv[1], "%d", &STEP_COUNT);
	//sscanf (argv[2], "%d", &ELEM_COUNT);

	test(STEP_COUNT, ELEM_COUNT);

	return 0;
}