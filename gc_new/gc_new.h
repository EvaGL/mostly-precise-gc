/*************************************************************************************//**
        * File: gc_new.h
        * Description: This file consists realiasation functions  from "gc_new.cpp"
        * Update: 20/10/13
*****************************************************************************************/
#pragma once
#include <cstdio>
#include <pthread.h>
#include <vector>
#include <cstdlib>
#include "../object_repres/go.h"
#include "../object_repres/boxing2.h"
#include <typeinfo>
#include <unordered_map>
#include <string>

extern pthread_mutex_t mut;  
extern std::vector <void *> offsets;
extern bool new_active;
extern std::vector <void *> ptr_in_heap;
extern std::unordered_map <std::string, void *> list_meta_obj;
extern size_t counter;

using std::make_pair;

/**
* @class meta information
* @brief stored significant for collecting information
* @detailed the first main thing that stored - pointer object box
*/
class base_meta
{
    public:
    void *shell; /**< pointer on the box(meta info struct for storing offsets) of object */
    size_t size; /**< size of object */
    bool mbit; /**< mark bit(garbage(set False) or alive(set True)) of object */        
    virtual void del_ptr () = 0; /**< delete meta-ptr */
    virtual void* get_begin () = 0; /**< get begin of object(pointer on meta)*/
};

/**
* @class base_meta  
* @brief the class with virtual functions from meta
* @detailed stored pointer on the beginning gc_ptr object(base_meta)
*/
template <class T> class meta : public base_meta
{
    public:
    T* ptr;
    
    void del_ptr () {
        if (size == 1) {
            ((T*)ptr)->~T();
        } else {
            for (size_t i = 0; i < size; i++)
                ((T*)ptr)[i].~T();
        }
    }

    void* get_begin () {
        return reinterpret_cast <void*> (this);
    }
};

/**
* @class gc_new
* @brief the class for allocating space and setting metainf
* @detailed for different kinds of call different allocating 
*/
template <class T> T* gc_new (size_t count=1)  
{
    while (pthread_mutex_trylock(&mut)!=0){};  /* trying to lock */

    counter += sizeof(T);  /* num of space that we used ++ */
    if (counter >= 5000000) {/* if occupated place more than 500000000 lets start to collect */
        mark_and_sweep();
        counter = 0;
    }

    void *res; /* ninitialize object which will be store the result */
    if (count <= 0)
        return 0;

    new_active = true;  /* set flag that object creates(allocates) in heap */
    offsets.clear();  /* clean from old offsets for new object */
    
   /*<allocating space and creating meta data for pointers
   *case: count == 1 --- pointer on simplle-type object
   *otherwise: other pointers(...)*/
    if (count == 1) {
        res = malloc(sizeof(T) + sizeof(void*) + sizeof(meta<T>));  /* allocate space */
        new ((char *)res + sizeof(void*) + sizeof(meta<T>)) T;  /* create object in allocated space, call gc_ptr constructor, get new struct offsets */
        *((size_t*)((char *)res + sizeof(meta<T>))) =  reinterpret_cast <size_t> (new (res) meta<T>);  /* initialize meta in obj */
        meta<T>* m_inf = reinterpret_cast <meta<T>* > (res);  /* stored pointer on meta */

        if (offsets.size() == 0) {  /* if num offsets inside object is empty */
            if (list_meta_obj.count(typeid(T).name()))  /* if type of this  meta stored in list_meta_obj */
                m_inf->shell = list_meta_obj[typeid(T).name()];  /* just save it in current shell */
            else { /*if doesn't */
                m_inf->shell = generic_box_simple (); /* create new type - box, save in shell */
                list_meta_obj[typeid(T).name()] = m_inf->shell; /* add in list with the same types */
            }
        } else { /*case offset count more than 0, doing the same things except created boxes */
            std::list <size_t> offsets_ptr;  
            for (size_t i = 0; i < offsets.size(); i++) {  /* getting all offsets_ptrs */
                offsets_ptr.push_front(reinterpret_cast <size_t> (offsets[i]) - reinterpret_cast <size_t> ((char *)res + sizeof(void*) + sizeof(meta<T>)));  /* getting pointers */
            }
            if (list_meta_obj.count(typeid(T).name()))  
                m_inf->shell = list_meta_obj[typeid(T).name()]; 
            else {
                m_inf->shell = generic_box_struct (offsets_ptr, sizeof(T), count); /*create new box and save pointer in shell */
                list_meta_obj[typeid(T).name()] = m_inf->shell;  
            }
        }
        m_inf->size = count;  /* count of offsets */
        m_inf->mbit = 0;
        m_inf->ptr = (T*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>)); /*set pointer on value of object(gc_ptr) */
    } else { 
        res = malloc(sizeof(T) * count + sizeof(void *) + sizeof(meta<T>));
        new ((char *)res + sizeof(void*) + sizeof(meta<T>)) T[count]; /* here we calling constructor from T[count] */
        *((size_t*)((char *)res + sizeof(meta<T>))) = reinterpret_cast <size_t> (new (res) meta<T>);
        meta<T>* m_inf = reinterpret_cast <meta<T>* > (res);                

        if (offsets.size() == 0) {
            if (list_meta_obj.count(typeid(T).name()))
                m_inf->shell = list_meta_obj[typeid(T).name()];
            else {
                m_inf->shell = generic_box_unboxed_array (count);
                list_meta_obj[typeid(T).name()] = m_inf->shell;
            }
        } else {
            std::list <size_t> offsets_ptr;
            for (size_t i = 0; i < offsets.size() / count; i++)
                offsets_ptr.push_front(reinterpret_cast <size_t> (offsets[i]) - reinterpret_cast <size_t> ((char *)res + sizeof(void*) + sizeof(meta<T>)));
            if (list_meta_obj.count(typeid(T).name()))
                m_inf->shell = list_meta_obj[typeid(T).name()];
            else {
                m_inf->shell = generic_box_struct (offsets_ptr, sizeof(T), count);
                list_meta_obj[typeid(T).name()] = m_inf->shell;
            }
        }
        m_inf->size = count;
        m_inf->mbit = 0;
        m_inf->ptr = (T*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>));
    }

    ptr_in_heap.push_back((char *)res + sizeof(base_meta*) + sizeof(meta<T>));  /* adding ptr in list of heap pointers */
    new_active = false;  
    offsets.clear();  /* finished - cleaning */
    pthread_mutex_unlock(&mut);  /* unlocking */

    return (T*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>));  /*return ptr on allocated space, begining value */
}