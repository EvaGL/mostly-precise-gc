//
// Created by evagl on 13.05.15.
//

#ifndef DIPLOMA_TLVARS_H
#define DIPLOMA_TLVARS_H
#include <vector>
#include <stddef.h>

/**
* @structure represents an element of class metainformation list
* @field name --- pointer to typeid(T).name();
* @field pointer --- pointer to class meta (offsets etc)
* @field next --- pointer on the next list element
*/
struct MetaInformation {
    const void * name;
    void * pointer;
    MetaInformation * next;
};

struct tlvars {
    std::vector<size_t> offsets;
    bool new_active = false;
    bool no_active = false;
    MetaInformation* classMeta = new MetaInformation();
    int nesting_level = 0;
    size_t current_pointer_to_object = 0;
};

extern thread_local new_object_flags new_obj_flags_tl_instance;
#endif //DIPLOMA_TLVARS_H
