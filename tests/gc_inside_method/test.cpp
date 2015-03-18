#include <libprecisegc/libprecisegc.h>

class B; // forward declaration

class A {
public:
    gc_ptr<B> b;
    A() {
        b = gc_new<B, int>(0x0000face);
    }
};

class B {
private:
    int value;
public:
    B(int v = 0xdeadbeef) : value(v) {}

    void test(gc_ptr<A> parent) {
        int v = value;
        parent->b.setNULL();
        gc();
        gc_new<A>();
        gc_new<A>();
        gc_new<A>();
        assert(v == value);
    }
};

int main() {
    gc_ptr<A> a = gc_new<A>();
    a->b->test(a);
    return 0;
}
