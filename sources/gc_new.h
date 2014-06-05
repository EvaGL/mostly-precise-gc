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
#include "taginfo.h"
#include <typeinfo>
#include "meta_information.h"
#include <vector>
#include <assert.h>
#include <msmalloc.h>

extern pthread_mutex_t mut;  
extern bool new_active;
extern bool no_active;
extern size_t counter;
extern std::vector <void *> offsets;
extern MetaInformation * classMeta;
extern int nesting_level;

/**
* @class meta information
* @brief stored significant for collecting information
* @detailed the first main thing that stored - pointer object box
*/
class base_meta {
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
template <class T> class meta : public base_meta {
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

template <class T> bool hasOffsets ( void ) {
    std::vector <void *> temp = offsets;
    offsets.clear();
    const char * typeidName = typeid(T).name();
    void * clMeta = contains(classMeta, typeidName);
    void * res = malloc(sizeof(T) + sizeof(void*) + sizeof(meta<T>));  /* allocate space */
    transfer_to_automatic_objects(res);
    new ((char *)res + sizeof(void*) + sizeof(meta<T>)) T();
    *((size_t*)((char *)res + sizeof(meta<T>))) =  reinterpret_cast <size_t> (new (res) meta<T>);  /* initialize meta in obj */
    meta<T>* m_inf = reinterpret_cast <meta<T>* > (res);  /* stored pointer on meta */
    
    if (offsets.empty()) {
        if (clMeta) {
            m_inf->shell = clMeta; 
        } else { /*if doesn't */
            m_inf->shell = generic_box_simple (); /* create new type - box, save in shell */
            addNewClassMetaInformation(typeidName, m_inf->shell);
        }
    } else { /*case offset count more than 0, doing the same things except created boxes */
        std::list <size_t> offsets_ptr;
        for (size_t i = 0; i < offsets.size(); i++) { /* getting all offsets_ptrs */
            offsets_ptr.push_front(reinterpret_cast <size_t> (offsets[i]) - reinterpret_cast <size_t> (res + sizeof(void*) + sizeof(meta<T>)));
        }
        if (clMeta) {
            m_inf->shell = clMeta;
        } else {
            m_inf->shell = generic_box_struct (offsets_ptr, sizeof(T), 1); /*create new box and save pointer in shell */
            addNewClassMetaInformation(typeidName, m_inf->shell);
        }
    }
    m_inf->size = 1;  /* count of offsets */
    m_inf->ptr = (T*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>)); /*set pointer on value of object(gc_ptr) */

    bool result = (!offsets.empty());
    offsets.clear(); /* free memory, occupied by offsets */
    offsets = temp; /* restore previous level offsets */
    temp.clear();
    return result;
}

/**
* @class gc_new
* @brief the class for allocating space and setting metainf
* @detailed for different kinds of call different allocating 
*/
template <class T, typename ... Types> T* gc_new (Types ... types, size_t count = 1) {
    counter += sizeof(T);  /* num of space that we used ++ */
    if (counter > 50000000 && nesting_level == 0) {/* if occupated place more than 50000000 lets start to collect */
        mark_and_sweep();
        counter = 0;
    }

    void *res = NULL; /* ninitialize object which will be store the result */
    assert(count >= 0);

    new_active = true;  /* set flag that object creates(allocates) in heap */
    nesting_level++;
    std::vector <void *> temp = offsets; /* save previous level offsets in temporary object */
    offsets.clear();
    const char * typeidName = typeid(T).name();
    void * clMeta = contains(classMeta, typeidName); // get pointer to class meta or NULL if it is no meta for this class
    meta<T>* m_inf = NULL;
    
    res = malloc(sizeof(T) * count + sizeof(void*) + sizeof(meta<T>));  /* allocate space */
    transfer_to_automatic_objects(res);
    if (count == 1) {
        new ((char *)res + sizeof(void*) + sizeof(meta<T>)) T(types ... );  /* create object in allocated space, call gc_ptr constructor, get new struct offsets */
        *((size_t*)((char *)res + sizeof(meta<T>))) =  reinterpret_cast <size_t> (new (res) meta<T>);  /* initialize meta in obj */
        m_inf = reinterpret_cast <meta<T>* > (res);  /* stored pointer on meta */

        if (offsets.empty()){
            if (clMeta) {
                m_inf->shell = clMeta; 
            } else { /*if doesn't */
                m_inf->shell = generic_box_simple (); /* create new type - box, save in shell */
                addNewClassMetaInformation(typeidName, m_inf->shell);
            }
        } else { /*case offset count more than 0, doing the same things except created boxes */
            std::list <size_t> offsets_ptr;
            for (size_t i = 0; i < offsets.size(); i++) { /* getting all offsets_ptrs */
                offsets_ptr.push_front(reinterpret_cast <size_t> (offsets[i]) - reinterpret_cast <size_t> (res + sizeof(void*) + sizeof(meta<T>)));
            }
            if (clMeta) {
                m_inf->shell = clMeta;
            } else {
                m_inf->shell = generic_box_struct (offsets_ptr, sizeof(T), count); /*create new box and save pointer in shell */
                addNewClassMetaInformation(typeidName, m_inf->shell);
            }
        }
    } else {
        no_active = true; // offsets wont counting yet
        new ((char *)res + sizeof(void *) + sizeof(meta<T>)) T[count];
        *((size_t*)((char *)res + sizeof(meta<T>))) = reinterpret_cast <size_t> (new (res) meta<T>);
        m_inf = reinterpret_cast <meta<T>* > (res);

        no_active = false; // offsets will counting yet
        new_active = true; // restore, because string new ... T[coint]; changed it to false
        // force counting class metainfo
        if (hasOffsets<T>()) {
            clMeta = contains(classMeta, typeidName);
            assert(clMeta != NULL);
            m_inf->shell = generic_box_boxed_array(count, clMeta, sizeof(T));
        } else {
            clMeta = contains(classMeta, typeidName);
            m_inf->shell = generic_box_unboxed_array(count);
        }
        assert(clMeta != NULL);
    }
    m_inf->size = count;  /* count of offsets */
    m_inf->ptr = (T*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>)); /*set pointer on value of object(gc_ptr) */
    new_active = false;

    offsets.clear(); /* free memory, occupied by offsets */
    offsets = temp; /* restore previous level offsets */
    temp.clear();
    nesting_level--;

    return (T*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>));  /*return ptr on allocated space, begining value */
}