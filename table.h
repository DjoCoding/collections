#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define __HASH_TABLE_SIZE 100

typedef size_t(*hashfuncT)(char *key);

#define Table(T) void ***

// `V`: Hash Table Item Value Type
// `hasher`: Custom Hash Function Of Type size_t(*hashfuncT)(char *key)
// `htinit` initializes the hash table.
#define htinit(V, hasher)           (__htinit(sizeof(V), (hashfuncT)(hasher)))

// `V`: Hash Table Item Value Type
// `table`: HashTable<V>
// `key`: key for mapping
// `htset` creates a new hash table item and returns a pointer to its value.
#define htset(V, table, key)        ((V *)(__htset(table, key)))

// `table`: HashTable<V>
// `key`: key for mapping
// `httryget` tries to get the item by key. if the key is not found, it will return NULL, else it returns the key value pair.
#define httryget(table, key)        (__httryget(table, key))

// `table`: HashTable<V>
// `key`: key for mapping
// `htget` gets the value of the key in the table. (It will raise an error if the key is not found in the table).
#define htget(V, table, key)        (*((V *)__htget(table, key)))

// `V`: Hash Table Item Value Type
// `kv`: KVPair<char *, V>
// `a` utility function to get the value of a KVPair.
#define htvalue(V, kv)              (*(V *)__htvalue(kv))

// `table`: HashTable<V>
// `htclean` cleans the table memory. 
#define htclean(table)              (__htclean(table))

typedef struct {
    char *key;
    void *value;
} KVPair;

void ***__htinit(size_t vtype, hashfuncT hasher);
void *__htset(void ***table, char *key);
void *__htget(void ***table, char *key);
void *__htvalue(KVPair *kv);
void  __htclean(void ***table);
KVPair *__httryget(void ***table, char *key);

#ifdef HASH_TABLE_IMPLEMENTATION

typedef struct {
    size_t vtype;
    hashfuncT hasher;
} TableHeader;

typedef struct TableItem {
    char *key;
    void *value;
    struct TableItem *next, *prev;
} TableItem;


// `murmur3_64` hash function 
size_t defhashfunc(char *key) {
    size_t len      = strlen(key);
    uint32_t seed   = 42;

    const uint8_t *data = (const uint8_t *)key;
    const int nblocks = len / 16;

    uint64_t h1 = seed;
    uint64_t h2 = seed;

    const uint64_t c1 = 0x87c37b91114253d5ULL;
    const uint64_t c2 = 0x4cf5ad432745937fULL;

    // `body`
    const uint64_t *blocks = (const uint64_t *)(data);
    for (int i = 0; i < nblocks; i++) {
        uint64_t k1 = blocks[i * 2 + 0];
        uint64_t k2 = blocks[i * 2 + 1];

        k1 *= c1; k1 = (k1 << 31) | (k1 >> 33); k1 *= c2; h1 ^= k1;
        h1 = (h1 << 27) | (h1 >> 37); h1 += h2; h1 = h1 * 5 + 0x52dce729;

        k2 *= c2; k2 = (k2 << 33) | (k2 >> 31); k2 *= c1; h2 ^= k2;
        h2 = (h2 << 31) | (h2 >> 33); h2 += h1; h2 = h2 * 5 + 0x38495ab5;
    }

    // `tail`
    const uint8_t *tail = (const uint8_t *)(data + nblocks * 16);
    uint64_t k1 = 0, k2 = 0;

    switch (len & 15) {
        case 15: k2 ^= (uint64_t)(tail[14]) << 48;
        case 14: k2 ^= (uint64_t)(tail[13]) << 40;
        case 13: k2 ^= (uint64_t)(tail[12]) << 32;
        case 12: k2 ^= (uint64_t)(tail[11]) << 24;
        case 11: k2 ^= (uint64_t)(tail[10]) << 16;
        case 10: k2 ^= (uint64_t)(tail[9]) << 8;
        case  9: k2 ^= (uint64_t)(tail[8]);
                 k2 *= c2; k2 = (k2 << 33) | (k2 >> 31); k2 *= c1; h2 ^= k2;

        case  8: k1 ^= (uint64_t)(tail[7]) << 56;
        case  7: k1 ^= (uint64_t)(tail[6]) << 48;
        case  6: k1 ^= (uint64_t)(tail[5]) << 40;
        case  5: k1 ^= (uint64_t)(tail[4]) << 32;
        case  4: k1 ^= (uint64_t)(tail[3]) << 24;
        case  3: k1 ^= (uint64_t)(tail[2]) << 16;
        case  2: k1 ^= (uint64_t)(tail[1]) << 8;
        case  1: k1 ^= (uint64_t)(tail[0]);
                 k1 *= c1; k1 = (k1 << 31) | (k1 >> 33); k1 *= c2; h1 ^= k1;
    }

    // `finalization`
    h1 ^= len;
    h2 ^= len;

    h1 += h2;
    h2 += h1;

    // `fmix64`
    h1 ^= h1 >> 33;
    h1 *= 0xff51afd7ed558ccdULL;
    h1 ^= h1 >> 33;
    h1 *= 0xc4ceb9fe1a85ec53ULL;
    h1 ^= h1 >> 33;

    h2 ^= h2 >> 33;
    h2 *= 0xff51afd7ed558ccdULL;
    h2 ^= h2 >> 33;
    h2 *= 0xc4ceb9fe1a85ec53ULL;
    h2 ^= h2 >> 33;

    h1 += h2;

    return (size_t)h1;
}

TableHeader *__header(size_t vtype, hashfuncT hasher) {
    // `one` additional pointer for the table.
    TableHeader *header = (TableHeader *)malloc(sizeof(TableHeader) + sizeof(void *));
    assert(header != NULL && "ERROR: malloc failed.");

    memset(header, 0, sizeof(TableHeader));
    header->vtype  = vtype;
    header->hasher = hasher == NULL ? defhashfunc : hasher;

    void ***table = (void ***)(header + 1);
    
    *table = malloc(sizeof(TableItem) * __HASH_TABLE_SIZE);
    assert(table != NULL && "ERROR: malloc failed.");

    memset(*table, 0, sizeof(TableItem) * __HASH_TABLE_SIZE);

    return header;
}

TableHeader *__headerfromht(void ***table) {
    return (TableHeader *)table - 1;
}

TableItem *__htitem(char *key, size_t vtype) {
    TableItem *item = (TableItem *)malloc(sizeof(TableItem));
    assert(item != NULL && "ERROR: malloc failed.");
    memset(item, 0, sizeof(TableItem));

    size_t klen = strlen(key);
    
    item->key = (char *)malloc(sizeof(char) * (klen + 1));
    assert(item->key != NULL && "ERROR: malloc failed.");
    memcpy(item->key, key, sizeof(char) * (klen + 1));

    item->value = malloc(vtype);
    assert(item->value != NULL && "ERROR: malloc failed.");
    memset(item->value, 0, vtype);

    return item;
}

void ***__htinit(size_t vtype, hashfuncT hasher) {
    TableHeader *header = __header(vtype, hasher);
    void ***table = (void ***)(header + 1);
    return table;
}

void *__htset(void ***table, char *key) {
    if(key == NULL) {
        fprintf(stderr, "Hash Table ERROR: NULL keys are not allowed.");
        exit(1);
    }

    TableHeader *header = __headerfromht(table);
    
    size_t index = header->hasher(key) % __HASH_TABLE_SIZE;
    TableItem **itemptr = &((TableItem **)(*table))[index];

    TableItem *current = *itemptr;
    while(current != NULL) {
        if(strcmp(current->key, key) == 0) {
            return current->value;
        }
        current = current->next;
    }

    TableItem *item = __htitem(key, header->vtype);

    if(*itemptr == NULL) {
        *itemptr = item;
        return item->value;
    }

    item->next = *itemptr;
    (*itemptr)->prev = item;

    return item->value;
}

KVPair *__httryget(void ***table, char *key) {
    if(key == NULL) {
        fprintf(stderr, "Hash Table ERROR: NULL keys are not allowed.");
        exit(1);
    }

    TableHeader *header = __headerfromht(table);
    
    size_t index = header->hasher(key) % __HASH_TABLE_SIZE;
    TableItem **itemptr = &((TableItem **)(*table))[index];

    TableItem *current = *itemptr;
    
    while(current != NULL) {
        if(strcmp(current->key, key) == 0) return (KVPair *)current;
        current = current->next;
    }

    return NULL;
}

void *__htget(void ***table, char *key) {
    KVPair *pair = __httryget(table, key);
    if(pair == NULL) {
        fprintf(stderr, "Hash Table ERROR: Expected key %s to have a value.", key);
        exit(1);
    }
    
    return pair->value;
}

void *__htvalue(KVPair *kv) {
    return kv->value;
}

void __htclean(void ***table) {
    TableHeader *header = __headerfromht(table);
    free(header);

    TableItem **items = (TableItem **)*table;
    for(size_t i = 0; i < __HASH_TABLE_SIZE; ++i) {
        TableItem *item = items[i];
        TableItem *current = item;
        while(current != NULL) {
            TableItem *next = current->next;
            free(current->key);
            free(current->value);
            free(current);
            current = next;
        }
    } 
}

#endif // `HASH_TABLE_IMPLEMENTATION`

#endif // `HASH_TABLE`