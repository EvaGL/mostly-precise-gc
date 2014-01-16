#pragma once
#include <stdio.h>

struct PointerList {
    void * pointer;
    PointerList * next;
};

PointerList * copyPointerList (PointerList * pointerList);
void addElemetToPointerList (PointerList * pointerList, void * ptr);
PointerList * pushBackToPointerList (PointerList * pointerList, void * ptr);
void clearPointerList (PointerList * pointerList);
size_t sizeOfPointerList (PointerList * pointerList);
void * getElementFromPointerList (PointerList * pointerList, int number);