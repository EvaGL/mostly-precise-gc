#pragma once

/**
* @structure represents an element of class metainformation list
* @field name --- hash key (type name)
* @field pointer --- pointer to class meta (offsets etc)
* @field next --- pointer on the next list element
*/
struct MetaInformation {
	char * name;
	void * pointer;
	MetaInformation * next;
};

/**
* @function checks is class name str contains in list of metainformation
* @return pointer on the class meta (second element of MetaInformation structure)
*	or NULL in the non-availability such element case 
*/
void * contains					(MetaInformation * meta, const char * str);
/**
* @function && @return the same as contains-func
*/
void * getClassMetaPointer		(MetaInformation * meta, const char * str);
/**
* @function adds new element in class meta list
* @params --- fields of new MetaInformation (structure) element
*/
void addNewClassMetaInformation	(const char * str, void * ptr);