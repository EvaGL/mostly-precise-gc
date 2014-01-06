/*************************************************************************************//**
        * File: gc_new.h
        * Description: This file consists realiasation functions  from "gc_new.cpp"
        * Update: 20/10/13
*****************************************************************************************/
#pragma once
#include <cstdio>
#include <pthread.h>
#include <vector>
//#include <cstdlib>
//#include "../object_repres/mark.h"

#include "../object_repres/go.h"
#include "../object_repres/boxing2.h"
#include <typeinfo>
#include <unordered_map>
#include <string>

#define DEBUGE_MODE true

extern pthread_mutex_t mut;  
//extern std::vector <void *> offsets;
extern bool new_active;
//extern std::vector <void *> ptr_in_heap;
//extern std::unordered_map <std::string, void *> list_meta_obj;
extern size_t counter;
extern "C" {
    void mark(void*);
}

using std::make_pair;

struct PointerList;
extern PointerList * offsets;
struct PointerList {
    void * pointer;
    PointerList * next;

    void markAll () {
        PointerList * temp = offsets;
        while (temp != NULL) {
            mark(temp);
            if (temp->pointer) {
                mark(temp->pointer);
            }
            temp = temp->next;
        }
    }

    void addElemet (void * ptr) {
        PointerList * temp = (PointerList *) malloc(sizeof(PointerList));
        temp->pointer = ptr;
        temp->next = offsets;
        offsets = temp;
    }

    void push_back (void * ptr) {
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

    void clear () {
        offsets = NULL;
    }

    size_t size () {
        PointerList * temp = offsets;
        int result = 0;
        while (temp != NULL) {
            result++;
            temp = temp->next;
        }
        return result;
    }

    void * getElement (int number) {
        PointerList * temp = offsets;
        for (int i = 0; i < number; i++, temp = temp->next) {
            if (!temp) return NULL;
        }
        return temp->pointer;
    }
};

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
    //bool mbit; /**< mark bit(garbage(set False) or alive(set True)) of object */        
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

class MetaInformation {
private:
    class MetaList {
    private:
        std::string className;
        void * pointerToClassMetaInfo;

    public:
        MetaList * next;

        MetaList () : className (""), pointerToClassMetaInfo (NULL), next (NULL) {}

        MetaList (std::string name, void * ptr, MetaList * next_el = NULL) : className (name), pointerToClassMetaInfo (ptr), next (next_el) {}

        void setClassName (std::string str) {
            this->className = str;
        }

        std::string getClassName () {
            return this->className;
        }

        void setPointerToClassMetaInfo (void * ptr) {
            this->pointerToClassMetaInfo = ptr;
        }

        void * getPointerToClassMetaInfo () {
            return this->pointerToClassMetaInfo;
        }
    };

    MetaList * list;

public:
    MetaInformation () {
        list = new MetaList();
    }

    void markMeta () {
        if (DEBUGE_MODE) {
            printf("markMeta\n");
            fflush(stdout);
        }
        mark(this);
        MetaList * temp = list;
        while (temp != NULL) {
            if (DEBUGE_MODE) {
                printf("\t ptr %p\n", temp->getPointerToClassMetaInfo());
                fflush(stdout);
            }            
            mark(temp);
            if (temp->getPointerToClassMetaInfo() != NULL) {
                mark(temp->getPointerToClassMetaInfo());
            }
            temp = temp->next;
        }
        if (DEBUGE_MODE) {
            printf("markMeta end\n");
            fflush(stdout);
        }
    }

    int count (std::string name) {
        MetaList * temp = this->list;
        int result = 0;
        while (temp != NULL) {
            if (temp->getClassName() == name) {
                result++;
            }
            temp = temp->next;
        }
        return result;
    }

    void * getClassMetaPointer (std::string name) {
        MetaList * temp = list;
        while (temp != NULL) {
            if (temp->getClassName() == name) {
                return temp->getPointerToClassMetaInfo();
            }
            temp = temp->next;
        }
        return NULL;
    }

    // set pointer and adds new element to the list
    void setClassMetaPointer (std::string name, void * pointer) {
        MetaList * temp = list;
        while (temp != NULL) {
            if (temp->getClassName() == name) {
                temp->setPointerToClassMetaInfo(pointer);
                return;
            }
            temp = temp->next;
        }
        MetaList * newElement = new MetaList(name, pointer, list);
        this->list = newElement;
        return;
    }
};

extern MetaInformation * list_meta_obj;

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
    if (counter > 0) {/* if occupated place more than 500000000 lets start to collect */
        if (DEBUGE_MODE) {
                printf("start makr and sweep\n");
                fflush(stdout);
        }
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
        
        if (DEBUGE_MODE) {
            printf("after malloc\n");
            fflush(stdout);
        }

        if (offsets->size() == 0) {  /* if num offsets inside object is empty */
            if (DEBUGE_MODE) {
                printf("if (offsets.size() == 0) {\n");
                fflush(stdout);
            }
            if (list_meta_obj->count(typeid(T).name()))  /* if type of this  meta stored in list_meta_obj */
                // m_inf->shell = list_meta_obj[typeid(T).name()];  /* just save it in current shell */
                m_inf->shell = list_meta_obj->getClassMetaPointer(typeid(T).name());
            else { /*if doesn't */
                m_inf->shell = generic_box_simple (); /* create new type - box, save in shell */
                //list_meta_obj[typeid(T).name()] = m_inf->shell; /* add in list with the same types */
                list_meta_obj->setClassMetaPointer(typeid(T).name(), m_inf->shell);
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
                printf("after for %p\n", &list_meta_obj);
                fflush(stdout);
                printf("after for %s\n", typeid(T).name());
                fflush(stdout);
            }
            if (list_meta_obj->count(typeid(T).name())) {
                if (DEBUGE_MODE) {
                    printf("if-1\n");
                    fflush(stdout);
                }               
                //m_inf->shell = list_meta_obj[typeid(T).name()]; 
                m_inf->shell = list_meta_obj->getClassMetaPointer(typeid(T).name());
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
                // printf("%i\n", list_meta_obj.count(typeid(T).name()));
                // fflush(stdout);
                // list_meta_obj[typeid(T).name()] = m_inf->shell;
                list_meta_obj->setClassMetaPointer(typeid(T).name(), m_inf->shell);
            }
        }
        if (DEBUGE_MODE) {
            printf("after-if-1\n");
            fflush(stdout);
        }    
        m_inf->size = count;  /* count of offsets */
        //m_inf->mbit = 0;
        if (DEBUGE_MODE) {
            printf("before\n        m_inf->ptr = (T*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>)); /*set pointer on value of object(gc_ptr) */ \n");
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
            if (list_meta_obj->count(typeid(T).name())) {
                //m_inf->shell = list_meta_obj[typeid(T).name()];
                m_inf->shell = list_meta_obj->getClassMetaPointer(typeid(T).name());
            } else {
                m_inf->shell = generic_box_unboxed_array (count);
                //list_meta_obj[typeid(T).name()] = m_inf->shell;
                list_meta_obj->setClassMetaPointer(typeid(T).name(), m_inf->shell);
            }
        } else {
            std::list <size_t> offsets_ptr;
            for (size_t i = 0; i < offsets->size() / count; i++)
                offsets_ptr.push_front(reinterpret_cast <size_t> (offsets->getElement(i)) - reinterpret_cast <size_t> ((char *)res + sizeof(void*) + sizeof(meta<T>)));
            if (list_meta_obj->count(typeid(T).name())) {
                //m_inf->shell = list_meta_obj[typeid(T).name()];
                m_inf->shell = list_meta_obj->getClassMetaPointer(typeid(T).name());
            } else {
                m_inf->shell = generic_box_struct (offsets_ptr, sizeof(T), count);
                //list_meta_obj[typeid(T).name()] = m_inf->shell;
                list_meta_obj->setClassMetaPointer(typeid(T).name(), m_inf->shell);
            }
        }
        m_inf->size = count;
        //m_inf->mbit = 0;
        m_inf->ptr = (T*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>));
    }

    //ptr_in_heap.push_back((char *)res + sizeof(base_meta*) + sizeof(meta<T>));  /* adding ptr in list of heap pointers */
    new_active = false;  
    offsets->clear();  /* finished - cleaning */
    pthread_mutex_unlock(&mut);  /* unlocking */

    if (DEBUGE_MODE) {
        printf("gc_new ends %p\n", (void*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>)));
        fflush(stdout);
    }

    return (T*)((char *)res + sizeof(base_meta*) + sizeof(meta<T>));  /*return ptr on allocated space, begining value */
}