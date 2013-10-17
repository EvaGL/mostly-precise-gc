/*************************************************************************************//**
        * File: go.h
        * Description: name of main functions in GC. Walking through the objects and deleting the objects.
        * Update: 17/10/13
*****************************************************************************************/

#pragma once

/**
 \fn go - walks through the objects
 \brief first arg - pointer on object, sec - mark_bit
 */
void go (void *v, bool mark_bit);

/**
 \fn mark_and_sweep - mark and delete garbage
 */
void mark_and_sweep (); 
