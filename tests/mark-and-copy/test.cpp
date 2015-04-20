//
// Created by evagl on 26.03.15.
//
#include <libprecisegc/libprecisegc.h>

struct A {
    A(int v=0xfaceface) : value(v) {}
    int value;
};

struct B {
    gc_ptr<A> a;
    B() {}
    B(gc_ptr<A> aa) : a(aa) {}
};

int main() {
    gc_ptr<A> a = gc_new<A, int>(0xdeadbeef);
    gc_ptr<B> b = gc_new<B, gc_ptr<A> >(a);
    A* ptr = a.get();
    int value = ptr->value;
    gc();
    assert(ptr != a.get());
    assert(value == a->value);
    assert(b->a.get() == a.get());
    return 0;
}