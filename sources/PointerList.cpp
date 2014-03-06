#include "PointerList.h"
#include <msmalloc.h>
#include <sys/mman.h>

PointerList * copyPointerList (PointerList * pointerList) {
    PointerList * newPointerList = NULL, * temp = pointerList;
    while (temp != NULL) {
        newPointerList = pushBackToPointerList(newPointerList, temp->pointer);
        temp = temp->next;
    }
    return newPointerList;
}

PointerList * addElemetToPointerList (PointerList * pointerList, void * ptr) {
    PointerList * temp = (PointerList *) mmap (0, sizeof(PointerList), PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    temp->pointer = ptr;
    temp->next = pointerList;
    return temp;
}

PointerList * pushBackToPointerList (PointerList * pointerList, void * ptr) {
    PointerList * newElement = (PointerList *) mmap (0, sizeof(PointerList), PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0), * temp = pointerList;
    newElement->pointer = ptr;
    newElement->next = NULL;
    if (temp == NULL) {
        pointerList = newElement;
        return pointerList;
    }
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = newElement;
    return pointerList;
}

PointerList * clearPointerList (PointerList * pointerList) {
    while (pointerList != NULL) {
        PointerList * temp = pointerList->next;
        munmap((void*)pointerList, sizeof(PointerList));
        pointerList = temp;
    }
    return NULL;
}

size_t sizeOfPointerList (PointerList * pointerList) {
    PointerList * temp = pointerList;
    int result = 0;
    while (temp != NULL) {
        result++;
        temp = temp->next;
    }
    return result;
}

void * getElementFromPointerList (PointerList * pointerList, int number) {
    PointerList * temp = pointerList;
    for (int i = 0; i < number; i++, temp = temp->next) {
        if (!temp) return NULL;
    }
    return temp->pointer;
}
