#ifndef _LEXER_H_
#define _LEXER_H_ 1

#include <stdlib.h>

enum LexerTokenType {
  TKN_NAME,
  TKN_NUMBER,
  TKN_STRING,
  TKN_COMMENT_ONENLINE,
  TKN_COMMENT_MULTILINE,
  TKN_COMMENT_DOCUMENTATION,
  TKN_ENDLINE,
  TKN_SYMBOL,
  TKN_OPERATION,
  TKN_COMMA,
  TKN_HASH,
  TKN_NEWLINE,
  TKN_SPACE,
  TKN_EOF,
  TKN_UNKNOWN,
};

struct LexerToken {
  char *string;
  enum LexerTokenType type;
  size_t start;
  size_t end;
};

struct Lexer {
  const char *string;
  const char *pos; // pointing to the last read position
  int eof;
};

void lexer_init(struct Lexer *, const char *);
void lexer_token_init(struct LexerToken *);
int lexer_next_token(struct Lexer *, struct LexerToken *);
const char *lexer_token_type_to_string(enum LexerTokenType);

#endif
