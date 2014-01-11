#include "mark.h"

void mal (void) {
	int i = 0;
	for (i = 0; i < 10000; i++) {
		void * temp = malloc (i);
	}
}

int main (void) {
	int i = 0;
	for (i = 0 ; i < 1000000; i++) {
		mal();
		sweep();
	}
	return 0;
}