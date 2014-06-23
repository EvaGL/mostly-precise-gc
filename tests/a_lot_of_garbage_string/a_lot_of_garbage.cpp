#include <iostream>
#include <stdio.h>
#include "GCString.h"

using std::cout;
using std::endl;

int array_size = 10;

class Node {
	gc_ptr<GCString> str;
public:
	gc_ptr<Node> left, right;

	Node() {}
	Node(gc_ptr<Node> l, gc_ptr<Node> r) {
		left = l; right = r;
	}
	Node(gc_ptr<Node> l, gc_ptr<Node> r, const char * sin) {
		left = l; right = r;
		str = gc_new<GCString, const char *>(sin);
	}
	Node(gc_ptr<Node> l, gc_ptr<Node> r, gc_ptr<char> sin) {
		left = l; right = r;
		str = gc_new<GCString, const char *>(sin);
	}

	void setString (gc_ptr<GCString> s) {
		str = s;
	}

	void print () {
		str->print();
	}
};

gc_ptr<Node> makeTree (int depth, gc_ptr<char> array) {
	if (depth == 0)
		return gc_new<Node>();
	else
		return gc_new<Node, gc_ptr<Node>, gc_ptr<Node>, gc_ptr<char>>
			(makeTree(depth - 1, array), makeTree(depth - 1, array), array);
}

gc_ptr<Node> createTree (int depth) {
	gc_ptr<char> mas;
	mas = gc_new<char>(array_size);
	for (int i = 0; i < array_size; i++) {
		mas[i] = i + '0';
	}
	return makeTree(depth, mas);
}

void print (gc_ptr<Node> node) {
	if (node.get() == NULL) {
		return;
	}
	node->print();
	print(node->left);
	print(node->right);
}

void changeTree (gc_ptr<Node> node, const char * str) {
	if (node.get() == NULL) return;

	gc_ptr<GCString> s = gc_new<GCString, const char *>(str);
	node->setString(s);

	changeTree(node->left, str);
	changeTree(node->right, str);
}

void func () {
	gc_ptr<Node> node;
	node = createTree(5);
	// print(node);
	mark_and_sweep();

	// print(node);
	changeTree(node, "qwertyyyyyy");
	mark_and_sweep();
	// print(node);
}

int main (void) {
	func();
	mark_and_sweep();

	return 0;
}