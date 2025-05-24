


#include "dynamicarray.h"



void dyanmicarray_init(DynamicArray *arr, int initialCapacity) {
    arr->data = malloc(initialCapacity * sizeof(void *));
    if (arr->data == NULL) {
        TraceLog(LOG_ERROR, "Failed to allocate initial memory for dynamic array");
        arr->count = 0;
        arr->capacity = 0;
        return;
    }
    arr->count = 0;
    arr->capacity = initialCapacity;
}



void dyanmicarray_add(DynamicArray *arr, void *element) {
    if (arr->count == arr->capacity) {
        // Resize the array: double the capacity
        int newCapacity = arr->capacity == 0 ? 1 : arr->capacity * 2;
        void **newData = realloc(arr->data, newCapacity * sizeof(void *));
        if (newData == NULL) {
            TraceLog(LOG_ERROR, "Failed to reallocate memory for dynamic array");
            return; // Handle reallocation failure (you might want to do more)
        }
        arr->data = newData;
        arr->capacity = newCapacity;
    }
    arr->data[arr->count] = element;
    arr->count++;
}



void dyanmicarray_remove(DynamicArray *arr, int index) {
    if (index < 0 || index >= arr->count) {
        TraceLog(LOG_WARNING, "Index out of bounds for dynamic array removal");
        return;
    }
    if (arr->count > 0) {
        // Shift elements after the removed index to the left
        for (int i = index; i < arr->count - 1; i++) {
            arr->data[i] = arr->data[i + 1];
        }
        arr->count--;
        // Optionally, you could shrink the capacity if the array becomes significantly smaller
        // to save memory.
    }
}


void dyanmicarray_free(DynamicArray *arr) {
    free(arr->data);
    arr->data = NULL;
    arr->count = 0;
    arr->capacity = 0;
}