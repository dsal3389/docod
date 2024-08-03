#include "dll.h"
#include <stdlib.h>
#include <string.h>

void dll_init(struct DoubleLinkedList *dll) {
  dll->head = NULL;
  dll->tail = NULL;
  dll->length = 0;
}

void dll_node_init(struct Node *node, void *value) {
  node->next = NULL;
  node->prev = NULL;
  node->value = value;
}

void dll_push(struct DoubleLinkedList *dll, struct Node *node) {
  node->next = NULL;
  node->prev = dll->tail;

  // if we don't have `head` we also don't have tail
  if (dll->tail == NULL) {
    dll->head = node;
  } else {
    dll->tail->next = node;
  }
  dll->tail = node;
  dll->length++;
}

struct Node *dll_pop(struct DoubleLinkedList *dll) {
  if (dll->tail == NULL) {
    return NULL;
  }
  if (dll->head == dll->tail) {
    dll->head = NULL;
  }

  struct Node *node = dll->tail;
  dll->tail = node->prev;

  if (dll->tail != NULL) {
    dll->tail->next = NULL;
  }
  return node;
}
