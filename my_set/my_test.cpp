#include <cstdio>
#include "gc2.h"

struct list
{
	int val;
	gc_ptr<list> next;

	list ()
	{
		val = 0;
		next = 0;
	}
};

class que {
public:
	que() {
		listHead = listBottom = 0;
	}

	gc_ptr<list> listHead;
	gc_ptr<list> listBottom;

	void push (int newValue) {
		gc_ptr<list> newEl = gc_new<list>(1);
		newEl->val = newValue;
		newEl->next = 0;


		printf("push %i", newValue);

		if (listBottom == 0) {
			listBottom = listHead = newEl;
		} else {
			listHead->next = newEl;
			listHead = listHead->next;
		}
	}

	void print() {
		for(gc_ptr<list> i = listBottom; i != 0; i = i->next) {
			//printf("%i ", i->val);
		}
		perror("end\n");
	}

};

int main (void)
{
	for(int i = 0; i < 500000; i++) {
		gc_ptr<que> newQue = gc_new<que>(1);
		for(int j = 0; j < 5000; j++) {
			newQue->push(j);
			printf(" after push ");
		}
		//newQue->print();
	}

	return 0;
}