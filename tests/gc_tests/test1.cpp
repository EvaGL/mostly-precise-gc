#include <libgc/libgc.h>
#include <math.h>

const int n = 5;

void produce_garbage (void) {
	gc_ptr<int> array;
	for (int i = 0; i < n; i++) {
		array = gc_new<int>(pow(2, i) * 100);
	}
}

int main (void) {
	produce_garbage();
	// force gc call!
	printf("force gc call\n");
	gc();
	return 0;
}