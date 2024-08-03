#ifndef _LEXER_H_
#define _LEXER_H_ 1

#include <stdlib.h>

enum LexerTokenType {
  TKN_NAME,
  TKN_NUMBER,
  TKN_STRING,
  TKN_KEYWORD,
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
  size_t len;
};

struct Lexer {
  const char *string;
  size_t cursor;
  int eof;
};

// initialize the `Lexer` struct
void lexer_init(struct Lexer *, const char *);

// initialize the `LexerToken` struct
void lexer_token_init(struct LexerToken *);

// initilize a lexer with the given file content
void lexer_file_content(struct Lexer *, const char *);

// gets the next token from the lexer and corrects the
// provided `LexerToken` with the information, if no token is parsed `0`
// is returned, otherwise '1'
int lexer_next_token(struct Lexer *, struct LexerToken *);

// converts `LexerTokenType` enum to string value
const char *lexer_token_type_to_string(enum LexerTokenType);

// returns the `LexerToken` string value based on the information
// found on the lexer, the returned char pointer points to a shared heap data
// that changes with each function call
const char *lexer_token_position_string(struct Lexer *, struct LexerToken *);

// like `lexer_token_position_string` but takes a buffer
const char *lexer_token_position_string_r(struct Lexer *, struct LexerToken *,
                                          char *);

#endif
