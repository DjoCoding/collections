#ifndef COLLECTIONS_TABLE_H
#define COLLECTIONS_TABLE_H

#include <stdio.h>
#include <stdbool.h>

void    *__table_create(size_t key_size, size_t value_size);
void    *__table_get(void *table, void *key);
void     __table_set(void *table, void *key, void *value);
void    __table_delete(void *table, void *key);
bool    __table_exists(void *table, void *key);
void    __table_rewind(void *table);
void    *__table_next(void *table);
void    __table_destroy(void *table);

typedef struct TableNode TableNode;

/**
 * @brief Macro used purely as a documentation placeholder and type alias
 * for a generic Table or Hash Map structure.
 * * This definition is not meant to provide type safety or structural information
 * to the compiler. It merely allows developers to write code that looks like:
 * 'Table(KeyType, ValueType) my_map;' for readability.
 * * In reality, the type resolves to a generic void pointer, and type safety
 * must be managed manually by the programmer using casts.
 * * @param K The type of the keys to be stored in the table.
 * @param V The type of the values to be stored in the table.
 * @return `void*` A generic pointer to the underlying table structure.
 */
#define Table(K, V)             void *


/**
 * @brief Creates a new hash table for key–value pairs of the given types.
 *
 * Allocates a hash table capable of storing keys and values of fixed sizes.
 * 
 * Example:
 * ```c
 * Table table = table_create(int, float);
 * ```
 *
 * @tparam K The key type.
 * @tparam V The value type.
 * @return Pointer to the start of the table data.
 */
#define table_create(K, V) ((Table(K, V))__table_create(sizeof(K), sizeof(V)))

/**
 * @brief Inserts or updates a key–value pair in the table.
 *
 * This macro allows passing the value directly (by copy) without manually taking its address.
 * If the key does not exist, a new entry is created. If it exists, the value is updated.
 *
 * Example:
 * ```c
 * table_set(table, "HELLO", 4);
 * table_set(table, key_str, 3.14f);
 * ```
 *
 * @param t Pointer to the table.
 * @param k Pointer to the key (or string literal).
 * @param v The value to store (passed by copy).
 */
#define table_set(t, k, v) \
    do { \
        typeof(v) __val = (v); \
        __table_set(t, (void *)(k), &__val); \
    } while (0)

/**
 * @brief Retrieves the value associated with a given key.
 *
 * Returns the value itself (not a pointer), making it easy to use directly in expressions.
 * If the key is not found, behavior is undefined (check with `table_exists` first).
 *
 * Example:
 * ```c
 * if (table_exists(table, "HELLO")) {
 *     int value = table_get(table, "HELLO", int);
 *     printf("%d\n", value);
 * }
 * ```
 *
 * @param t Pointer to the table.
 * @param k Pointer to the key (or string literal).
 * @param T The expected value type.
 * @return The value associated with the key.
 */
#define table_get(T, t, k) (*(T *)__table_get(t, (void *)(k)))


/**
 * @brief Deletes a key–value entry from the hash table. 
 *
 * Finds the entry associated with the given key and removes it from the table.
 * Note that this macro performs a generic cast to facilitate calling the 
 * underlying implementation function.
 *
 * Example:
 * ```c
 * // Assume 'user_table' stores char* keys.
 * table_delete(user_table, "alice");
 * ```
 *
 * @param t The pointer to the hash table (Table type).
 * @param k The key of the entry to be deleted. The macro casts this to void*.
 */
#define table_delete(t, k)   (__table_delete(t, (void *)k))

/**
 * @brief Checks whether a key exists in the table.
 *
 * Example:
 * ```c
 * if (table_exists(table, "HELLO")) {
 *     printf("Key found!\n");
 * }
 * ```
 *
 * @param t Pointer to the table.
 * @param k Pointer to the key (or string literal).
 * @return `true` if the key exists, otherwise `false`.
 */
#define table_exists(t, k) (__table_exists(t, (void *)(k)))


/**
 * @brief Resets the iteration cursor to the beginning of the table.
 *
 * This function performs the action of **rewinding** the internal table cursor, 
 * setting both the bucket index (cy) and the entry index (cx) to zero. This 
 * prepares the table for a fresh iteration over all its key-value pairs.
 *
 * Example:
 * ```c
 * // Prepare for a full table scan iteration
 * table_rewind(my_table_ptr); 
 * ```
 *
 * @param t The pointer to the hash table (Table type).
 */
#define table_rewind(t)     (__table_rewind(t))

// ---

/**
 * @brief Advances the iteration cursor to the next valid entry in the table.
 *
 * This function is used to iterate through all key-value pairs. It moves the 
 * cursor (cx, cy) to the next element, handling collision chain traversal (cx increment)
 * and moving to the next bucket (cy increment) when a chain is exhausted.
 *
 * Example:
 * ```c
 * Entry *next_entry = table_next(Entry, my_table_ptr);
 * if (next_entry != NULL) { ... }
 * ```
 *
 * @tparam T The expected type of the next entry (e.g., Entry*, KV*, etc.). This 
 * macro is designed to return a typed pointer for the next element.
 * @param t The pointer to the hash table (Table type).
 * @return A typed pointer to the next valid key-value entry, or NULL if the 
 * iteration has reached the end of the table.
 */
#define table_next(T, t)    ((T *)__table_next(t))


/**
 * @brief Destroys the table and frees all associated memory.
 *
 * After this call, the pointer becomes invalid and must not be used.
 *
 * Example:
 * ```c
 * table_destroy(table);
 * table = NULL;
 * ```
 *
 * @param t Pointer to the table.
 */
#define table_destroy(t) (__table_destroy(t))


#ifdef COLLECTIONS_TABLE_IMPLEMENTATION

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_CAPACITY 100

/**
 * @brief Structure containing the essential metadata for a generic hash table.
 *
 * This header defines the properties of the data stored in the table and holds
 * temporary state information for iteration and cursor management.
 */
typedef struct {
    /** The fixed size, in bytes, of the key for all entries in the table. */
    size_t  key_size;

    /** The fixed size, in bytes, of the value for all entries in the table. */
    size_t  value_size;

    /** * The cursor's Y-coordinate (cy). 
     * In hash tables, this typically represents the current **bucket index** * being inspected within the main array.
     */
    size_t  cy;
    
    /** * The cursor's X-coordinate (cx). 
     * In hash tables, this typically represents the **position** (index) of the 
     * current entry within the **collision chain** (linked list) of the bucket.
     */
    size_t  cx;
} TableHeader;

typedef uint64_t HASH;

struct TableNode {
    void        *key;
    void        *value;
    TableNode   *next;
};

#define tableheader(p)          ((TableHeader *)p - 1)

static inline uint64_t __hash(const void *data, size_t len) {
    const unsigned char *bytes = (const unsigned char *)data;
    uint64_t hash = 1469598103934665603ull;  // FNV offset basis
    const uint64_t prime = 1099511628211ull; // FNV prime

    for (size_t i = 0; i < len; i++) {
        hash ^= bytes[i];
        hash *= prime;
    }

    return hash;
}

size_t __table_get_index__(void *table, void *key) {
    TableHeader *header = tableheader(table);
    HASH hash = __hash(key, header->key_size);
    return hash % TABLE_CAPACITY;
}

TableNode *__table_node_create__(void *table, void *key, void *value) {
    TableHeader *header = tableheader(table);

    TableNode *node = (TableNode *)malloc(sizeof(TableNode) + header->key_size + header->value_size);
    if(!node) {
        fprintf(stderr, "__table_set failed: cannot allocate memory\n");
        exit(EXIT_FAILURE);
    }

    node->key = (char *)node + sizeof(TableNode);
    memcpy(node->key, key, header->key_size);

    node->value = (char *)node + sizeof(TableNode) + header->key_size;
    memcpy(node->value, value, header->value_size);

    node->next = NULL;
    return node;
}


void *__table_create(size_t key_size, size_t value_size) {
    void *p = calloc(1, sizeof(TableHeader) + TABLE_CAPACITY * sizeof(TableNode *));
    if(!p) {
        fprintf(stderr, "__table_create failed: cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    TableHeader *header = (TableHeader *)p;
    header->key_size    = key_size;
    header->value_size  = value_size;

    header->cx = 0;
    header->cy = 0;

    void *data = (void *)(header + 1);
    return data;
}

void *__table_get(void *table, void *key) {
    TableHeader *header = tableheader(table);
    size_t index = __table_get_index__(table, key);

    TableNode *head = ((TableNode **)table)[index];
    if(!head) {
        fprintf(stderr, "__table_get failed: key not found.\n");
        exit(EXIT_FAILURE);
    };

    TableNode *current = head;
    while(current != NULL) {
        if(memcmp(current->key, key, header->key_size) == 0) break;
        current = current->next;
    }

    if(!current) {
        fprintf(stderr, "__table_get failed: key not found.\n");
        exit(EXIT_FAILURE);
    };

    return current->value;
}

void __table_set(void *table, void *key, void *value) {
    TableHeader *header = tableheader(table);
    size_t index = __table_get_index__(table, key);

    TableNode *head = ((TableNode **)table)[index];
    TableNode *current = head;

    while(current != NULL) {
        if(memcmp(current->key, key, header->key_size) == 0) break;
        current = current->next;
    }

    if(!current) {
        TableNode *node = __table_node_create__(table, key, value);
        node->next = ((TableNode **)table)[index];
        ((TableNode **)table)[index] = node;
        return;
    };

    memcpy(current->value, value, header->value_size);
}

void __table_delete(void *table, void *key) {
    TableHeader *header = tableheader(table);
    size_t index = __table_get_index__(table, key);

    TableNode *prev    = NULL;
    TableNode *current = ((TableNode **)table)[index];
    while(current != NULL) {
        if(memcmp(current->key, key, header->key_size) == 0) break;
        current = current->next;
    }

    if(!current) return;

    prev->next = current->next;
    free(current);
}

bool __table_exists(void *table, void *key) {
    return __table_get(table, key) != NULL;
}

void __table_rewind(void *table) {
    TableHeader *header = tableheader(table);
    header->cx = 0;
    header->cy = 0;    
}

void *__table_next(void *table) {
    TableHeader *header = tableheader(table);
    TableNode *current = ((TableNode **)table)[header->cy];

    size_t i = 0;
    while(current != NULL && i < header->cx) {
        current = current->next;
        i += 1;
    }

    if(header->cx != 0 || header->cy != 0)  current = (current != NULL) ? current->next : NULL;

    if(current != NULL) {
        header->cx += 1;
        return current->value;
    };

    header->cy += 1;
    current = ((TableNode **)table)[header->cy]; 

    while(current == NULL) {
        if(header->cy >= TABLE_CAPACITY) break;
        
        current = ((TableNode **)table)[header->cy];
        if(current != NULL) break;

        header->cy += 1;
    }

    if(current != NULL) {
        header->cx = 0;
        return current->value;
    }

    return NULL;
}

void __table_destroy(void *table) {
    TableHeader *header = tableheader(table);
    for(size_t i = 0; i < TABLE_CAPACITY; ++i) {
        TableNode *head = ((TableNode **)table)[i];
        TableNode *current = head;
        while(current != NULL) {
            TableNode *next = current->next;
            free(current);
            current = next;
        }
    }
    free(header);
}


#endif // COLLECTIONS_TABLE_IMPLEMENTATION


#endif // COLLECTIONS_TABLE_H