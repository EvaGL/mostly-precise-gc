#include <cstdlib>
#include <string>
#include <libprecisegc/libprecisegc.h>
#include "List.h"
#include "Tree.h"

static int array_size = 2;
bool debug_print = true;

void test () {
	gc_ptr<int> mas =  gc_new<int>(array_size);
	for (int i = 0; i < array_size; i++) {
		mas[i] = rand() % 30;
	}

	gc_ptr<List> list = gc_new<List, gc_ptr<int>, int>(mas, array_size);
	while (rand() % 50 != 1) {
		dprintf ("1\n"); fflush (stdout);
		list->insert(mas, array_size);
		dprintf ("2\n"); fflush (stdout);
		list->tree_sort();
		dprintf ("3\n"); fflush (stdout);
		// gc();
	}

	dprintf ("count: %i \n", list->count()); fflush (stdout);
	// mark_and_sweep();
	dprintf ("count: %i \n", list->count()); fflush (stdout);
	// gc();
	list = list->tree_sort();
	// gc();
}

int main (int argc, char * argv[]) {
	// for (int i = 0; i < argc; i++) {
	// 	std::string arg = argv[i];
	// 	if (arg.length() == 9 && arg.compare(0, 8, "dprintf=") == 0) {
	// 		debug_print = (arg.compare(8, 1, "1") == 0);
	// 	}
	// }
	 for (int i = 0; i < 100; i++) {
		test();
	 }
	// gc();
	return 0;
}
