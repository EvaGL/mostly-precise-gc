#include <libgc/libgc.h>

class A {
	gc_ptr<int> a1;
	gc_ptr<int> a2;
	gc_ptr<int> a3;
	gc_ptr<int> a4;

public:
	A () {
		a1 = gc_new<int>(1);
		a2 = gc_new<int>(1);
		a3 = gc_new<int>(1);
		a4 = gc_new<int>(1);
	}
};

class B {
	gc_ptr<A> a1;
	gc_ptr<A> a2;
	gc_ptr<A> a3;
	gc_ptr<A> a4;

public:
	B () {
		a1 = gc_new<A>();
		a2 = gc_new<A>();
		a3 = gc_new<A>();
		a4 = gc_new<A>();
	}
};


int main (void) {
	gc_ptr<B> b = gc_new<B>();
	// gc_ptr<A> a = gc_new<A>();
	gc();
	return 0;
}
