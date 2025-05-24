

#ifndef DYNAMICARRAY_H
#define WORLDOBJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <raylib.h> 

typedef struct {
    void **data;    
    int count;      
    int capacity;
} DynamicArray;


void dyanmicarray_init(DynamicArray *arr, int initialCapacity);
void dyanmicarray_add(DynamicArray *arr, void *element);
void dyanmicarray_remove(DynamicArray *arr, int index);
void dyanmicarray_free(DynamicArray *arr);

#endif