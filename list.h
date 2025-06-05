#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

typedef struct LinkedListNode LinkedListNode;
typedef LinkedListNode *listnode;

#define List(T)             T **
#define ListItem(T)         T *

// T: List Item Type
// llinit initializes and return a list of element type T.
#define llinit(T)           ((List(T))(__llinit(sizeof(T))))

// ll: List<T>, T: List Item Type
// llappend creates a node and return a pointer to its value (T *)
#define llappend(T, ll)     ((ListItem(T))(__llappend((void **)ll)))


// ll: List<T>
// llclean de-allocates the list data from memory
#define llclean(ll)         (__llclean((void **)ll))

// ll: List<T>
// llcount returns the number of the items in the list
#define llcount(ll)         (__llcount((void **)ll))

// ll: List<T>
// llreset resets an iterator to its first state.
#define llreset(ll)         (__llreset((void **)ll))

// ll: List<T>, T: List Item Type
// llget returns the data of the iterator
#define llget(T, ll)        ((ListItem(T))(__llget((void **)ll)))

// ll: List<T>, T: List Item Type
// llget returns the node of the iterator
#define llgetnode(ll)       (__llgetnode((void **)ll))

// ll: List<T>, T: List Item Type
// llnext gets the current iterator data and updates it to its next state.
// if the list runs out of items, llnext will return NULL.
#define llnext(T, ll)       ((ListItem(T))(__llnext((void **)ll)))

// ll: List<T>
// llnextnode gets the current iterator node and updates it to its next state.
// if the list runs out of items, llnext will return NULL.
#define llnextnode(ll)      (__llnextnode((void **)ll))


// ll: List<T>
// llend returns whether the iterator got to its end state.
#define llend(ll)           (__llend((void **)ll))

// node: listnode<T>, T: List Item Type
// llnodedata returns the data of the given node
#define llnodedata(T, node) (*((ListItem(T))__llnodedata(node)))

// node: listnode<T>
// llnodeprev returns the previous node of the given node in the list
#define llnodeprev(T, node) (__llnodeprev(node))

// node: listnode<T>
// llnodenext returns the next node of the given node in the list
#define llnodenext(T, node) (__llnodenext(node))

struct LinkedListNode {
    void *data;
    struct LinkedListNode *next, *prev;
};

typedef struct {
    size_t count;
    size_t type;
    listnode curr;
} LinkedListHeader;

LinkedListHeader *__llheader(size_t type) {
    // 2 additional pointers allocated for the head and the tail of the linked list
    LinkedListHeader *header = (LinkedListHeader *)malloc(sizeof(LinkedListHeader) + 2 * sizeof(void *));
    assert(header != NULL && "ERROR: malloc failed.");
    memset(header, 0, sizeof(LinkedListHeader));
    memset(header + 1, 0, 2 * sizeof(void *));
    header->type = type;
    return header;
}

LinkedListHeader *__headerfromll(void **ll) {
    return (LinkedListHeader *)ll - 1;
}

listnode __llnodeinit(size_t type) {
    listnode node = (listnode )malloc(sizeof(LinkedListNode));
    assert(node != NULL && "ERROR: malloc failed.");
    
    memset(node, 0, sizeof(LinkedListNode));

    node->data = (void *)malloc(type);
    assert(node->data != NULL && "ERROR: malloc failed.");

    memset(node->data, 0, type);

    return node;
} 

void **__llinit(size_t type) {
    LinkedListHeader *header = __llheader(type);
    return (void **)(header + 1);
}

void *__llappend(void **ll) {
    LinkedListHeader *header = __headerfromll(ll);
    void **head = ll;
    void **tail = ll + 1;

    listnode node = __llnodeinit(header->type);

    header->count += 1;

    if(*head == NULL) {
        *head = node;
        *tail = node;
        return node->data;
    }

    ((listnode )*tail)->next = node;
    node->prev = (listnode )*tail;
    *tail = node;

    return node->data;
}

void __llreset(void **ll) {
    __headerfromll(ll)->curr = (listnode )*ll;
}

bool __llend(void **ll) {
    return __headerfromll(ll)->curr == NULL;
}

listnode __llgetnode(void **ll) {
    return __headerfromll(ll)->curr;
}


void *__llget(void **ll) {
    listnode node = __llgetnode(ll);
    if(node == NULL) return NULL;
    return node->data;
}

void *__llnodedata(listnode node) {
    return node->data;
}

listnode __llnodenext(listnode node) {
    return node->next;
}

listnode __llnodeprev(listnode node) {
    return node->prev;
}

listnode __llnextnode(void **ll) {
    if(__llend(ll)) return NULL;
    
    LinkedListHeader *header    = __headerfromll(ll);
    listnode curr = header->curr;
    header->curr = header->curr->next;

    return curr;    
}

void *__llnext(void **ll) {
    if(__llend(ll)) return NULL;
    
    void *data = __llget(ll);

    LinkedListHeader *header    = __headerfromll(ll);
    header->curr = header->curr->next;

    return data;
}

size_t __llcount(void **ll) {
    return __headerfromll(ll)->count;
}

void __llclean(void **ll) {
    listnode head = (listnode )*ll;
    
    listnode current = head;
    while(current != NULL) {
        listnode next = current->next;
        free(current->data);
        free(current);
        current = next;
    }

    LinkedListHeader *header = __headerfromll(ll);
    header->count = 0;
    free(header);

    *ll = NULL;
}


#endif