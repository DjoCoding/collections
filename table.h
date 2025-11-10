#ifndef COLLECTIONS_TABLE_H
#define COLLECTIONS_TABLE_H

#include <stdio.h>
#include <stdbool.h>

void    *__table_create(size_t key_size, size_t value_size);
void    *__table_get(void *table, void *key);
void     __table_set(void *table, void *key, void *value);
bool    __table_exists(void *table, void *key);
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
#define table_create(K, V) ((Table)__table_create(sizeof(K), sizeof(V)))

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

typedef struct {
    size_t key_size;
    size_t value_size;
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
    void *p = malloc(sizeof(TableHeader) + TABLE_CAPACITY * sizeof(TableNode *));
    if(!p) {
        fprintf(stderr, "__table_create failed: cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    TableHeader *header = (TableHeader *)p;
    header->key_size    = key_size;
    header->value_size  = value_size;

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

bool __table_exists(void *table, void *key) {
    return __table_get(table, key) != NULL;
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