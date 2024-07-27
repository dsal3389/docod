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

void dll_push(struct DoubleLinkedList *dls, struct Node *node) {
  node->next = NULL;
  node->prev = dls->tail;

  // if we don't have `head` we also don't have tail
  if (dls->tail == NULL) {
    dls->head = node;
  } else {
    dls->tail->next = node;
  }
  dls->tail = node;
  dls->length++;
}

struct Node *dll_pop(struct DoubleLinkedList *dls) {
  if (dls->tail == NULL) {
    return NULL;
  }
  if (dls->head == dls->tail) {
    dls->head = NULL;
  }

  struct Node *node = dls->tail;
  dls->tail = node->prev;

  if (dls->tail != NULL) {
    dls->tail->next = NULL;
  }
  return node;
}
