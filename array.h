#ifndef ARRAY_H
#define ARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

typedef void (*arrforeachcbT)(void *);
typedef bool (*arrfiltercbT)(void *);

#define Array(T)                              T **
#define ArrayItem(T)                          T *

#define arrinit(T)                          (Array(T))(__arrinit(sizeof(T)))
#define arrappend(T, arr)                   (ArrayItem(T))(__arrappend((void **)(arr)))
#define arrcount(arr)                       (__arrcount((void **)(arr)))
#define arrgetp(T, arr, idx)                ((ArrayItem(T))__arrgetp((void **)(arr), idx))
#define arrget(T, arr, idx)                 (*(ArrayItem(T))__arrgetp((void **)(arr), idx))
#define arrclean(arr)                       (__arrclean((void **)(arr)))
#define arrforeach(arr, cb)                 (__arrforeach((void **)(arr), (arrforeachcbT)(cb)))
#define arrfilter(T, arr, cb)               (Array(T))(__arrfilter((void (**))(arr), (arrfiltercbT)(cb)))


typedef struct {
    size_t count;
    size_t size;
    size_t type;
} ArrayHeader;

ArrayHeader *__arrheader(size_t type) {
    // another pointer allocated that points to the array data
    ArrayHeader *header = (ArrayHeader *)malloc(sizeof(ArrayHeader) + sizeof(void *));
    assert(header != NULL && "ERROR: malloc failed");
    
    memset(header, 0, sizeof(*header));
    memset((void *)(header + 1), 0, sizeof(void *));
    
    header->type = type;
    return header;
}

ArrayHeader *__headerfromarr(void **arr) {
    return (ArrayHeader *)arr - 1;
}

void **__arrinit(size_t type) {
    ArrayHeader *header = __arrheader(type);
    void **base = (void **)(header + 1);
    return base;
}

void *__arrappend(void **arr) {
    ArrayHeader *header = __headerfromarr(arr);
    
    if(header->count >= header->size) {
        if(header->size == 0) header->size = 10; 
        header->size *= 2;
        *arr = realloc(*arr, header->size * header->type);
        assert(*arr != NULL && "ERROR: realloc failed");
    }

    size_t offset   = header->type * header->count++;
    char *itemptr   = (char *)*arr + offset;

    return (void *)itemptr;
}

size_t __arrcount(void **arr) {
    return __headerfromarr(arr)->count;
}

size_t __Arrayype(void **arr) {
    return __headerfromarr(arr)->type;
}

void *__arrgetp(void **arr, size_t idx) {
    ArrayHeader *header = __headerfromarr(arr);

    if(idx >= header->count) {
        fprintf(stderr, "collections::array => Index out of bounds\n");
        exit(1);
    }

    size_t offset = header->type * idx;
    return (void *)((char *)*arr + offset);
}

void __arrclean(void **arr) {
    ArrayHeader *header = __headerfromarr(arr);
    free(header);
    free(*arr);
}

void __arrforeach(void **arr, arrforeachcbT cb) {
    size_t count = __arrcount(arr);
    for (size_t i = 0; i < count; ++i) {
        void *item = __arrgetp(arr, i);
        cb(item);
    }
}

void **__arrfilter(void **arr, arrfiltercbT cb) {
    size_t count = __arrcount(arr);
    size_t type  = __Arrayype(arr);

    void **result = __arrinit(type);

    for(size_t i = 0; i < count; ++i) {
        void *item = __arrgetp(arr, i);
        if(!cb(item)) continue;
        memcpy(__arrappend(result), item, type);
    }

    return result;
}

#endif