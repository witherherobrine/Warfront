


#include "dynamicarray.h"


// allocate dynamic array with initial size
void dyanmicarray_init(DynamicArray *arr, int initialCapacity) {
    // 1. Validate input pointer: Always check for NULL array pointer first.
    if (arr == NULL) {
        TraceLog(LOG_ERROR, "DYNAMICARRAY: dyanmicarray_init called with NULL array pointer.");
        return; // Cannot initialize a NULL pointer
    }

    // 2. Validate initialCapacity: Handle negative or zero capacity gracefully.
    // A capacity of 0 is valid, as realloc will act like malloc on first add.
    // However, we should still handle the malloc(0) behavior explicitly.
    if (initialCapacity < 0) {
        TraceLog(LOG_WARNING, "DYNAMICARRAY: Initial capacity (%d) cannot be negative. Setting to 0.", initialCapacity);
        initialCapacity = 0;
    }

    // 3. Initialize members to a safe, known state before potential allocation failure.
    // This is crucial. If malloc fails, arr should still be in a valid, empty state.
    arr->data = NULL;
    arr->count = 0;
    arr->capacity = 0;

    // 4. Attempt allocation only if initialCapacity is greater than 0.
    // malloc(0) behavior is implementation-defined; it might return NULL or a valid, zero-sized pointer.
    // Explicitly handling it this way makes the behavior predictable.
    if (initialCapacity > 0) {
        arr->data = malloc(initialCapacity * sizeof(void *));
        if (arr->data == NULL) {
            // Log the error but leave arr in its initialized empty state.
            TraceLog(LOG_ERROR, "DYNAMICARRAY: Failed to allocate initial memory for dynamic array with capacity %d.", initialCapacity);
            // arr->count and arr->capacity are already 0, and arr->data is NULL, which is the desired state on failure.
            return;
        }
        // If allocation successful, set the capacity.
        arr->capacity = initialCapacity;
    }

}

void* dyanmicarray_get(const DynamicArray* arr, int index) {
    if (arr == NULL) {
        TraceLog(LOG_ERROR, "DYNAMICARRAY_GET: Attempted to get from a NULL array.");
        return NULL;
    }
    if (index < 0 || index >= arr->count) {
        TraceLog(LOG_WARNING, "DYNAMICARRAY_GET: Index %d out of bounds (0-%d).", index, arr->count - 1);
        return NULL; // Or handle as an error, e.g., assert, exit
    }
    return arr->data[index];
}

void dyanmicarray_clear(DynamicArray* arr) {
    if (arr == NULL) {
        TraceLog(LOG_ERROR, "DYNAMICARRAY_CLEAR: Attempted to clear a NULL array.");
        return;
    }
    TraceLog(LOG_DEBUG, "DYNAMICARRAY_CLEAR: Clearing array %p. Old count: %d.", (void*)arr, arr->count);
    arr->count = 0;
}

// add element to dynamic array
void dyanmicarray_add(DynamicArray *arr, void *element) {
    // 1. Validate input pointer: Check for NULL array pointer.
    if (arr == NULL) {
        TraceLog(LOG_ERROR, "DYNAMICARRAY: dyanmicarray_add called with NULL array pointer. Element not added.");
        return;
    }

    // 2. Check if reallocation is needed
    if (arr->count == arr->capacity) {
        // Calculate new capacity. Handle the initial case where capacity is 0.
        int newCapacity = arr->capacity == 0 ? 1 : arr->capacity * 2;

        // Attempt reallocation
        void **newData = realloc(arr->data, newCapacity * sizeof(void *));
        if (newData == NULL) {
            // Log critical error and return. The array state remains unchanged (full).
            TraceLog(LOG_FATAL, "DYNAMICARRAY: Failed to reallocate memory for array %p to new capacity %d. Element not added.",
                     (void*)arr, newCapacity);
            // Optionally: You might want to set a global error flag or handle this more severely
            // if memory exhaustion is a critical failure point for your application.
            return;
        }

        // Update array's data pointer and capacity
        arr->data = newData;
        arr->capacity = newCapacity;
    }

    // 3. Add the element
    // Defensive check: Even though it should be handled by capacity, a final check against
    // an unexpected state where count somehow exceeds capacity.
    if (arr->count >= arr->capacity) {
        TraceLog(LOG_ERROR, "DYNAMICARRAY: Internal error: count (%d) >= capacity (%d) after reallocation. Element not added.",
                 arr->count, arr->capacity);
        return;
    }

    arr->data[arr->count] = element;
    arr->count++;

    TraceLog(LOG_DEBUG, "DYNAMICARRAY: Added element %p to array %p. New count: %d.",
             element, (void*)arr, arr->count);
}


void dyanmicarray_remove(DynamicArray *arr, void *element) {
    if (arr == NULL) {
        TraceLog(LOG_ERROR, "DYNAMICARRAY: dyanmicarray_remove called with NULL array pointer.");
        return;
    }
    if (element == NULL) {
        TraceLog(LOG_WARNING, "DYNAMICARRAY: dyanmicarray_remove called with NULL element. Nothing to remove.");
        return;
    }

    int found_index = -1;
    for (int i = 0; i < arr->count; ++i) {
        if (arr->data[i] == element) {
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        // Shift elements down to fill the gap
        for (int i = found_index; i < arr->count - 1; ++i) {
            arr->data[i] = arr->data[i + 1];
        }
        arr->count--;
        TraceLog(LOG_DEBUG, "DYNAMICARRAY: Removed element %p from array %p. New count: %d.",
                 element, (void*)arr, arr->count);
    } else {
        TraceLog(LOG_WARNING, "DYNAMICARRAY: Element %p not found in array %p. No removal.",
                 element, (void*)arr);
    }
    // Optional: Shrink capacity if count is much smaller than capacity (e.g., < 25% full)
    // This is a more advanced optimization for later.
}
/* 
//remove element from dynamic array
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
} */

//free memory from dynamic array
void dyanmicarray_free(DynamicArray *arr) {
    free(arr->data);
    arr->data = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

//"clear" the dynamic array by just resetting it's count
void dyanmicarray_resetCount(DynamicArray *arr){
	arr->count = 0;
}








