#pragma once
#include <stdio.h>

struct PointerList {
    void * pointer;
    PointerList * next;
};

PointerList * copyPointerList (PointerList * pointerList);
PointerList * addElemetToPointerList (PointerList * pointerList, void * ptr);
PointerList * pushBackToPointerList (PointerList * pointerList, void * ptr);
PointerList * clearPointerList (PointerList * pointerList);
size_t sizeOfPointerList (PointerList * pointerList);
void * getElementFromPointerList (PointerList * pointerList, int number);