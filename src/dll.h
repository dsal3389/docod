#ifndef _DLL_H_
#define _DLL_H_ 1

#include <stdlib.h>

struct Node {
  const void *value;
  struct Node *next;
  struct Node *prev;
};

struct DoubleLinkedList {
  struct Node *head;
  struct Node *tail;
  size_t length;
};

// initilize double linked list struct
void dll_init(struct DoubleLinkedList *);

// initilize double linked list node
void dll_node_init(struct Node *, void *);

// pop item from given double linked list and returns
// to the caller, if list is empty, returns NULL
struct Node *dll_pop(struct DoubleLinkedList *);

// push given node to the list, if the node was allocated on the
// heap, it is the caller responsibility to free it
void dll_push(struct DoubleLinkedList *, struct Node *);

#endif
