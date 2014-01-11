/*************************************************************************************//**
        * File: gc_new.h
        * Description: This file consists realiasation functions  from "gc_new.cpp"
        * Update: 20/10/13
*****************************************************************************************/
#pragma once
#include <cstdio>
#include <pthread.h>
#include "go.h"
#include "boxing2.h"
#include <typeinfo>
#include "meta_information.h"
#include "PointerList.h"

#define DEBUGE_MODE false

extern pthread_mutex_t mut;  
extern bool new_active;
extern size_t counter;
extern PointerList * offsets;
extern MetaInformation * classMeta;

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
    if (DEBUGE_MODE) {
        printf("gc_new starts\n");
        fflush(stdout);
    }
    while (pthread_mutex_trylock(&mut)!=0){};  /* trying to lock */

    counter += sizeof(T);  /* num of space that we used ++ */
    if (counter > 500) {/* if occupated place more than 50000000 lets start to collect */
        mark_and_sweep();
        counter = 0;
    }

    void *res; /* ninitialize object which will be store the result */
    if (count <= 0) {
        return 0;
    }

    new_active = true;  /* set flag that object creates(allocates) in heap */
    offsets->clear();  /* clean from old offsets for new object */
    
    /*<allocating space and creating meta data for pointers
    *case: count == 1 --- pointer on simplle-type object
    *otherwise: other pointers(...)*/
    if (count == 1) {
        if (DEBUGE_MODE) {
            printf("if (count == 1) {\n");
            fflush(stdout);
        }
        res = malloc(sizeof(T) + sizeof(void*) + sizeof(meta<T>));  /* allocate space */
        new ((char *)res + sizeof(void*) + sizeof(meta<T>)) T;  /* create object in allocated space, call gc_ptr constructor, get new struct offsets */
        *((size_t*)((char *)res + sizeof(meta<T>))) =  reinterpret_cast <size_t> (new (res) meta<T>);  /* initialize meta in obj */
        meta<T>* m_inf = reinterpret_cast <meta<T>* > (res);  /* stored pointer on meta */
        
        if (offsets->size() == 0) {  /* if num offsets inside object is empty */
            if (DEBUGE_MODE) {
                printf("if (offsets.size() == 0) {\n");
                fflush(stdout);
            }
            if (contains(classMeta, typeid(T).name())) {
                m_inf->shell = getClassMetaPointer(classMeta, typeid(T).name()); 
            } else { /*if doesn't */
                m_inf->shell = generic_box_simple (); /* create new type - box, save in shell */
                addNewClassMetaInformation(typeid(T).name(), m_inf->shell);
            }
        } else { /*case offset count more than 0, doing the same things except created boxes */
            if (DEBUGE_MODE) {
                printf("        } else { /*case offset count more than 0, doing the same things except created boxes */\n");
                fflush(stdout);
            }
            std::list <size_t> offsets_ptr;
            for (size_t i = 0; i < offsets->size(); i++) {  /* getting all offsets_ptrs */
                if (DEBUGE_MODE) {
                    printf("in for\n");
                    fflush(stdout);
                }
                offsets_ptr.push_front(reinterpret_cast <size_t> (offsets->getElement(i)) - reinterpret_cast <size_t> ((char *)res + sizeof(void*) + sizeof(meta<T>)));  /* getting pointers */
            }
            if (DEBUGE_MODE) {
                fflush(stdout);
                printf("after for %s\n", typeid(T).name());
                fflush(stdout);
            }
            if (contains(classMeta, typeid(T).name())) {
                if (DEBUGE_MODE) {
                    printf("if-1\n");
                    fflush(stdout);
                }               
                m_inf->shell = getClassMetaPointer(classMeta, typeid(T).name());
            } else {
                if (DEBUGE_MODE) {
                    printf("else-1\n");
                    fflush(stdout);
                }                
                m_inf->shell = generic_box_struct (offsets_ptr, sizeof(T), count); /*create new box and save pointer in shell */
                if (DEBUGE_MODE) {
                    printf("after m_inf->shell = generic_box_struct (offsets_ptr, sizeof(T), count);\n");
                    fflush(stdout);
                } 
                addNewClassMetaInformation(typeid(T).name(), m_inf->shell);
            }
        }
        if (DEBUGE_MODE) {
            printf("after-if-1\n");
            fflush(stdout);
        }    
        m_inf->size = count;  /* count of offsets */
        if (DEBUGE_MODE) {
            printf("before m_inf->ptr = (T*)((char *)res ... \n");
            fflush(stdout);
        }
        m_inf->ptr = (T*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>)); /*set pointer on value of object(gc_ptr) */
    } else { 
        if (DEBUGE_MODE) {
            printf("} else {\n");
            fflush(stdout);
        }

        res = malloc(sizeof(T) * count + sizeof(void *) + sizeof(meta<T>));
        new ((char *)res + sizeof(void*) + sizeof(meta<T>)) T[count]; /* here we calling constructor from T[count] */
        *((size_t*)((char *)res + sizeof(meta<T>))) = reinterpret_cast <size_t> (new (res) meta<T>);
        meta<T>* m_inf = reinterpret_cast <meta<T>* > (res);                

        if (DEBUGE_MODE) {
            printf("after malloc---2\n");
            fflush(stdout);
        }

        if (offsets->size() == 0) {
            if (contains(classMeta, typeid(T).name())) {
                m_inf->shell = getClassMetaPointer(classMeta, typeid(T).name());
            } else {
                m_inf->shell = generic_box_unboxed_array (count);
                addNewClassMetaInformation(typeid(T).name(), m_inf->shell);
            }
        } else {
            std::list <size_t> offsets_ptr;
            for (size_t i = 0; i < offsets->size() / count; i++)
                offsets_ptr.push_front(reinterpret_cast <size_t> (offsets->getElement(i)) - reinterpret_cast <size_t> ((char *)res + sizeof(void*) + sizeof(meta<T>)));
            if (contains(classMeta, typeid(T).name())) {
                m_inf->shell = getClassMetaPointer(classMeta, typeid(T).name());
            } else {
                m_inf->shell = generic_box_struct (offsets_ptr, sizeof(T), count);
                addNewClassMetaInformation(typeid(T).name(), m_inf->shell);
            }
        }
        m_inf->size = count;
        m_inf->ptr = (T*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>));
    }

    new_active = false;  
    offsets->clear();  /* finished - cleaning */
    pthread_mutex_unlock(&mut);  /* unlocking */

    if (DEBUGE_MODE) {
        printf("gc_new ends %p \n", (void*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>)));
        fflush(stdout);
    }

    return (T*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>));  /*return ptr on allocated space, begining value */
}