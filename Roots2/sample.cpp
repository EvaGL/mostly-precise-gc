#include "gc_ptr.h"
#include <vector>
#include <cstdio>

int main () {
	gc_ptr <int> a;
	a = new int[20];
	gc_ptr <int> b;
	b = a;

	gc_ptr <double> d;

	d = new double;

	gc_ptr <float> e;

	e = new float;

	ptr_list *roots = collect();

	for (ptr_list *v = roots; v != 0; v = v->next)
		printf ("%p ", v->ptr);

	return 0;
}



