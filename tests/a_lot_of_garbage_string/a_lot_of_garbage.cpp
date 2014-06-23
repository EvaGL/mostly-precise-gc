#include <iostream>
#include <libgc/libgc.h>
#include <string.h>
#include <stdio.h>

using std::cout;
using std::endl;

int array_size = 100;

class Object {
public:
	gc_ptr<char> mas;
	int size;

	Object() {}
	Object(gc_ptr<char> m) : size(array_size) {
		mas = gc_new<char>(array_size);
		for (int i = 0; i < array_size; i++) {
			mas[i] = m[i];
		}
	}

	void print () {
		if (mas.get() == NULL)
			return;
		cout << "array: ";
		for (int i = 0; i < size; i++) {
			cout << mas[i] << " ";
		}
		cout << endl;
	}
};

class Node {
public:
	gc_ptr<Object> obj;
	gc_ptr<Node> left, right;

	Node() {}
	Node(gc_ptr<Node> l, gc_ptr<Node> r) {
		left = l; right = r;
	}
	Node(gc_ptr<Node> l, gc_ptr<Node> r, gc_ptr<char> mas) {
		left = l; right = r;
		obj = gc_new<Object, gc_ptr<char>>(mas);
	}

	void print () {
		if (obj.get() != NULL)
			obj->print();
	}
};

gc_ptr<Node> makeTree (int depth, gc_ptr<char> array) {
	if (depth == 0)
		return gc_new<Node>();
	else
		return gc_new<Node, gc_ptr<Node>, gc_ptr<Node>, gc_ptr<char>>(makeTree(depth-1, array), makeTree(depth-1, array), array);
}

void print (gc_ptr<Node> node) {
	if (node.get() == NULL) {
		return;
	}
	node->print();
	print(node->left);
	print(node->right);
}

gc_ptr<Node> createTree (int depth) {
	gc_ptr<char> mas;
	mas = gc_new<char>(array_size);
	for (int i = 0; i < array_size; i++) {
		mas[i] = i + '0';
	}
	return makeTree(depth, mas);
}

void changeTree (gc_ptr<Node> node, const char * str) {
	if (node.get() == NULL) return;
	if (node->obj.get() == NULL) return;
	if (node->obj->mas.get() == NULL) return;

	int str_size = strlen(str);
	node->obj->mas = gc_new<char>(array_size + str_size);
	node->obj->size = array_size + str_size;
	for (int i = 0; i < array_size; i++) {
		node->obj->mas[i] = 'a' + i;
	}
	for (int i = array_size, j = 0; i < array_size + str_size; i++, j++) {
		node->obj->mas[i] = str[j];
	}
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
	// mark_and_sweep();
	print(node);
}

int main (void) {
	func();
	mark_and_sweep();

	return 0;
}