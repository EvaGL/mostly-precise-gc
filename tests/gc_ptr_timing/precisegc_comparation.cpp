//
// Created by danya on 26.05.15.
//

#include "precisegc_comparation.h"
#include "timing.h"

void gc_ptr_linked_list (void) {
    long tStart, tFinish;
    gc_ptr<LinkedList<int>> ll = gc_new<LinkedList<int>>();

    std::cout << "GC_PTR pointers: list liseof " << N << " elements:" << std::endl;

    std::cout << "\tCreating        ";
    tStart = currentTime();
    for (int i = 0; i < N; i++) { ll->push_begin(i); }
    tFinish = currentTime();
    std::cout << elapsedTime(tFinish - tStart) << " msec" << std::endl;

    std::cout << "\tCalculating     ";
    // std::cout << ll->size_of() << std::endl;
    tStart = currentTime();
    size_t ll_size = ll->size_of();
    tFinish = currentTime();
    std::cout << elapsedTime(tFinish - tStart) << " msec" << std::endl;
    // std::cout << elapsedTime(tFinish - tStart) << " msec and is equal to " << ll_size << std::endl;

    std::cout << "\tSort N/100 els  ";
    // ll->print();
    tStart = currentTime();
    ll->stupid_bubble_sort(N / 100);
    tFinish = currentTime();
    std::cout << elapsedTime(tFinish - tStart) << " msec" << std::endl;
    // ll->print();

    // ll->print();
    std::cout << "\tReversing       ";
    tStart = currentTime();
    ll->reverse();
    tFinish = currentTime();
    std::cout << elapsedTime(tFinish - tStart) << " msec" << std::endl;
    // ll->print();

    std::cout << "\tClearing        ";
    tStart = currentTime();
    ll->clear();
    tFinish = currentTime();
    std::cout << elapsedTime(tFinish - tStart) << " msec" << std::endl;
}

void gc_ptr_vector (void) {
    long tStart, tFinish;
    std::vector<gc_ptr<Obj<int>>> gc_ptr_vector(N);
    std::cout << "GC_PTR_vector sizeof " << N << " (int) elements:" << std::endl;

    std::cout << "\tCreating        ";
    tStart = currentTime();
    for (int i = 0; i < N; i++) {
        std::cout << "\t " << i << std::endl;
        gc_ptr<Obj<int> > t = gc_new<Obj<int>, int>(i);
        gc_ptr_vector.push_back(t);
    }
    tFinish = currentTime();
    std::cout << elapsedTime(tFinish - tStart) << " msec" << std::endl;

    std::cout << "\tSort            ";
    tStart = currentTime();
    std::sort(gc_ptr_vector.begin(), gc_ptr_vector.end(), [](const gc_ptr<Obj<int>> l, const gc_ptr<Obj<int>> r)->bool{return l->operator<(*r);});
    tFinish = currentTime();
    std::cout << elapsedTime(tFinish - tStart) << " msec" << std::endl;

    std::cout << "\tSort            ";
    tStart = currentTime();
    std::sort(gc_ptr_vector.begin(), gc_ptr_vector.end(), [](const gc_ptr<Obj<int>> l, const gc_ptr<Obj<int>> r)->bool{return r->operator<(*l);});
    tFinish = currentTime();
    std::cout << elapsedTime(tFinish - tStart) << " msec" << std::endl;

    std::cout << "\tClearing        ";
    tStart = currentTime();
   // for (int i = 0; i < N; i++) { delete gc_ptr_vector[i]; }
    tFinish = currentTime();
    std::cout << elapsedTime(tFinish - tStart) << " msec" << std::endl;
}

void gc_ptr_timing (void) {
    long tStartAll, tFinishAll;
    tStartAll = currentTime();
    gc_ptr_linked_list();
    gc_ptr_vector();
    tFinishAll = currentTime();
    std::cout << "full test took  " << elapsedTime(tFinishAll - tStartAll) << " msec\n\n" << std::endl;
}

int main (void) {
    gc_ptr_timing();
    return 0;
}