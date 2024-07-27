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

void dll_init(struct DoubleLinkedList *);
void dll_node_init(struct Node *, void *);
struct Node *dll_pop(struct DoubleLinkedList *);
void dll_push(struct DoubleLinkedList *, struct Node *);

#endif
