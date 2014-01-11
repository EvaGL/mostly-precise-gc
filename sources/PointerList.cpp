#include "PointerList.h"
#include "../dlmalloc/mark.h"

extern PointerList * offsets;

void PointerList::addElemet (void * ptr) {
    PointerList * temp = (PointerList *) malloc(sizeof(PointerList));
    temp->pointer = ptr;
    temp->next = offsets;
    offsets = temp;
}

void PointerList::push_back (void * ptr) {
    PointerList * newElement = (PointerList *) malloc(sizeof(PointerList)), * temp = offsets;
    newElement->pointer = ptr;
    newElement->next = NULL;
    if (temp == NULL) {
        offsets = newElement;
        return;
    }
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = newElement;
}

void PointerList::clear () {
    offsets = NULL;
}

size_t PointerList::size () {
    PointerList * temp = offsets;
    int result = 0;
    while (temp != NULL) {
        result++;
        temp = temp->next;
    }
    return result;
}

void * PointerList::getElement (int number) {
    PointerList * temp = offsets;
    for (int i = 0; i < number; i++, temp = temp->next) {
        if (!temp) return NULL;
    }
    return temp->pointer;
}