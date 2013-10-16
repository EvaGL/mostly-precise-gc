/****************************************************************************************
        * File: go.h
        * Description: name of main functions in GC. Walking through the objects and deleting the objects.
        * Update: 17/10/13
*****************************************************************************************/

#pragma once
void go (void *v, bool mark_bit); /*< walk through the obj*/
void mark_and_sweep (); /*< mark and delete garbage*/
