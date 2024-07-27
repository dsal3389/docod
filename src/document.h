#ifndef _DOCUMENT_H_
#define _DOCUMENT_H_ 1

#include "lexer.h"

struct DocSymbol {};

struct DocContext {
  const char *file_path;
  struct Lexer *lxr;
};

#endif
