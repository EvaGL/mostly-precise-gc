#pragma once
#include "tlvars.h"
/**
* @function checks is class name str contains in list of metainformation
* @return pointer on the class meta (second element of MetaInformation structure)
*	or NULL in the non-availability such element case 
*/
void * contains					(MetaInformation * meta, const void * name);

/**
* @function && @return the same as contains-func
*/
void * getClassMetaPointer		(MetaInformation * meta, const void * name);

/**
* @function adds new element in class meta list
* @params --- fields of new MetaInformation (structure) element
*/
void addNewClassMetaInformation	(tlvars * flags, const void * name, void * ptr);