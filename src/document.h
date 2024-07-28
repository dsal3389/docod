#ifndef _DOCUMENT_H_
#define _DOCUMENT_H_ 1

#include "dll.h"
#include "lexer.h"

struct DocumentItem {
  const char *title;
  const char *description;
};

struct DocumentCtx {
  const char *path;
  struct Lexer *lxr;
  struct DoubleLinkedList items;
};

void doc_init(struct DocumentCtx *, const char *, struct Lexer *);

#endif
