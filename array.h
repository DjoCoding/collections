#ifndef COLLECTIONS_ARRAY_H
#define COLLECTIONS_ARRAY_H


#include <stdio.h>

void        *__array_at(void *arr, size_t idx);
void        *__array_create(size_t item_size);
void        *__array_append(void *arr);
void        *__array_clear(void *arr);
void        *__array_pop(void *arr);
size_t      __array_length(void *arr);
void        __array_destroy(void *arr);


/**
 * @brief Creates a new dynamic array for elements of type `T`.
 *
 * Internally allocates an `ArrayHeader` followed by space for elements of type `T`.
 *
 * Example:
 * ```c
 * int *arr = array_create(int);
 * ```
 *
 * @tparam T The element type.
 * @return Pointer to the start of the array data.
 */
#define array_create(T)             ((T *)(__array_create(sizeof(T))))

/**
 * @brief Appends a new element to the given array.
 *
 * Expands the underlying storage if necessary. The pointer returned may differ
 * from the original if reallocation occurs.
 *
 * Example:
 * ```c
 * arr = array_append(int, arr);
 * arr[array_length(arr) - 1] = 42;
 * ```
 *
 * @tparam T The element type.
 * @param p  Pointer to the array.
 * @return Updated pointer to the array data.
 */
#define array_append(T, p)          ((T *)(__array_append(p)))

/**
 * @brief Clears all elements in the given array.
 *
 * Resets the logical length of the array to zero but preserves allocated capacity.
 *
 * Example:
 * ```c
 * array_clear(int, arr);
 * ```
 *
 * @tparam T The element type.
 * @param p  Pointer to the array.
 * @return Pointer to the cleared array.
 */
#define array_clear(T, p)           ((T *)(__array_clear(p)))

/**
 * @brief Retrieves the element at a specific index.
 *
 * Example:
 * ```c
 * int value = array_at(int, arr, 3);
 * ```
 *
 * @tparam T The element type.
 * @param p  Pointer to the array.
 * @param idx Index of the element (0-based).
 * @return The element value at the specified index.
 */
#define array_at(T, p, idx)         (*((T *)__array_at(p, idx)))

/**
 * @brief Removes and returns the last element of the array.
 *
 * Decreases the logical length of the array by one and returns the removed value.
 * 
 * Example:
 * ```c
 * int last = array_pop(int, arr);
 * ```
 *
 * @tparam T The element type.
 * @param p  Pointer to the array.
 * @return The element that was removed from the end of the array.
 */
#define array_pop(T, p)             (*((T *)__array_pop(p)))

/**
 * @brief Returns the number of elements in the array.
 *
 * Example:
 * ```c
 * size_t len = array_length(arr);
 * ```
 *
 * @param p Pointer to the array.
 * @return The logical length (element count) of the array.
 */
#define array_length(p)             (__array_length(p))

/**
 * @brief Destroys the array and frees its allocated memory.
 *
 * After calling this, the pointer becomes invalid and must not be used.
 *
 * Example:
 * ```c
 * array_destroy(arr);
 * arr = NULL;
 * ```
 *
 * @param p Pointer to the array.
 */
#define array_destroy(p)            (__array_destroy(p))

#ifdef COLLECTIONS_ARRAY_IMPLEMENTATION

#include <stdlib.h>

#define ARRAY_INITIAL_CAPACITY 10

typedef struct {
    size_t  length;
    size_t  cap;
    size_t  item_size;
} ArrayHeader;

/**
 * @brief Retrieves the array header from a data pointer.
 *
 * This macro assumes that the array data is stored immediately after
 * an `ArrayHeader` structure in memory. Given a pointer to the array’s
 * data (e.g., `T *p`), it returns a pointer to the associated
 * `ArrayHeader` by subtracting one element from the pointer.
 *
 * Example:
 * ```c
 * ArrayHeader *h = hft(array_data);
 * size_t length = h->length;
 * ```
 *
 * @param p Pointer to the start of the array data.
 * @return Pointer to the `ArrayHeader` structure associated with the array.
 */
#define hft(p) ((ArrayHeader *)(p) - 1)

void *__array_create(size_t item_size) {
    void **p = malloc(sizeof(ArrayHeader) + ARRAY_INITIAL_CAPACITY * item_size);
    if(!p) {
        fprintf(stderr, "__array_create failed: cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    ArrayHeader *header = (ArrayHeader *)p;
    header->item_size   = item_size;
    header->cap         = ARRAY_INITIAL_CAPACITY;
    header->length      = 0;

    void *data = (void *)(header + 1);
    return data;
}

void *__array_append(void *arr) {
    ArrayHeader *header = hft(arr);
    
    if(header->length < header->cap) {
        header->length += 1;
        return arr;
    };

    header->cap *= 2;
    
    void *p = realloc(header, sizeof(ArrayHeader) + header->cap * header->item_size);
    if(!p) {
        fprintf(stderr, "__array_append: cannot resize array.\n");
        exit(EXIT_FAILURE);
    };

    header = (ArrayHeader *)p;
    header->length += 1;

    void *data = (void *)(header + 1);
    return data;
}

void *__array_clear(void *arr) {
    ArrayHeader *header = hft(arr);
    header->length = 0;
    return arr;
}

void *__array_pop(void *arr) {
    ArrayHeader *header = hft(arr);
    if(header->length == 0) {
        fprintf(stderr, "__array_pop failed: array is empty.\n");
        exit(EXIT_FAILURE);
    }
    
    header->length -= 1;

    void *item = (void *)((char *)arr + header->item_size * header->length);
    return item;
}

void *__array_at(void *arr, size_t idx) {
    ArrayHeader *header = hft(arr);
    if(idx >= header->length) {
        fprintf(stderr, "__array_at failed: index out of range.\n");
        exit(EXIT_FAILURE);
    }
    return ((char *)arr + header->item_size * idx);
}

size_t __array_length(void *arr) {
    ArrayHeader *header = hft(arr);
    return header->length;
}

void __array_destroy(void *arr) {
    ArrayHeader *header = hft(arr);
    free(header);
}

#endif // COLLECTIONS_ARRAY_IMPLEMENTATION


#endif // COLLECTIONS_ARRAY_H