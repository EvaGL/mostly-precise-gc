#pragma once
#include <stdio.h>

struct PointerList {
    void * pointer;
    PointerList * next;

    void addElemet (void * ptr);
    void push_back (void * ptr);
    void clear ();
    size_t size ();
    void * getElement (int number);
};