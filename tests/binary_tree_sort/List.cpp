#include "List.h"
#include "Tree.h"

ListElement::ListElement (int i, gc_ptr<ListElement> nex) {
	value = i;
	next = nex;
}

gc_ptr<ListElement> ListElement::getNext (void) {
	return next;
}

void ListElement::print (void) {
	dprintf("%i ", value);
}

int ListElement::getValue (void) {
	return value;
}

List::List (gc_ptr<int> mas, int n) {
	for (int i = 0; i < n; i++) {
		this->insert(mas[i]);
	}
}

void List::insert (int i) {
	gc_ptr <ListElement> newEl = gc_new<ListElement, int, gc_ptr<ListElement>>(i, head);
	head = newEl;
}

void List::insert (gc_ptr<int> mas, int n) {
	for (int i = 0; i < n; i++) {
		this->insert(mas[i]);
	}
}

void List::print () {
	gc_ptr<ListElement> temp = this->head;
	while (temp.get() != NULL) {
		temp->print();
		temp = temp->getNext();
	}
	dprintf("\n");
}

int List::count (void) {
	gc_ptr<ListElement> temp = this->head;
	int res = 0;
	while (temp.get() != NULL) {
		res++;
		temp = temp->getNext();
	}
	return res;
}

gc_ptr<List> List::tree_sort () {
	gc_ptr<Tree> tree = gc_new<Tree>();
	gc_ptr<ListElement> tmp = head;
	while (tmp.get() != NULL) {
		tree->insert(tmp->getValue());
		tmp = tmp->getNext();
	}
	dprintf ("21\n"); fflush (stdout);
	gc_ptr<List> res = tree->toList();
	dprintf ("22\n"); fflush (stdout);
	res->print();
	dprintf ("23\n"); fflush (stdout);
	dprintf ("sorted!\n"); fflush (stdout);
	return res;
}
