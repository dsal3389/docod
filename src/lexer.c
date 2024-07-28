#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

// a macro to easily set token information
#define __LEXER_SET_TOKEN(tknptr, type_, start_, len_)                         \
  do {                                                                         \
    tknptr->type = type_;                                                      \
    tknptr->start = start_;                                                    \
    tknptr->len = len_;                                                        \
  } while (0);

static char *token_string_buffer = NULL;
static char *c_keywords[] = {
    "const",    "void",   "char",     "int",    "long",   "short",
    "struct",   "if",     "else",     "do",     "while",  "for",
    "continue", "break",  "return",   "float",  "double", "typedef",
    "typeof",   "signed", "unsigned", "switch", "case",   "default",
    "sizeof",   "goto",   "enum",     "union",  "inline", "register",
    "restrict", NULL};

static int lexer_name_is_keyword(const char *);
static inline const char *lexer_get_at_position(struct Lexer *, size_t, size_t);
static inline const char *lexer_get_at_position_r(struct Lexer *, size_t,
                                                  size_t, char *);

void lexer_init(struct Lexer *lxr, const char *string) {
  lxr->string = string;
  lxr->cursor = 0;
  lxr->eof = 0;
}

void lexer_token_init(struct LexerToken *tkn) {
  tkn->type = TKN_UNKNOWN;
  tkn->start = 0;
  tkn->len = 0;
}

#define __LEXER_TYPE_TO_STRING(t)                                              \
  case t:                                                                      \
    return #t;

const char *lexer_token_type_to_string(enum LexerTokenType type) {
  switch (type) {
    __LEXER_TYPE_TO_STRING(TKN_KEYWORD)
    __LEXER_TYPE_TO_STRING(TKN_NAME)
    __LEXER_TYPE_TO_STRING(TKN_STRING)
    __LEXER_TYPE_TO_STRING(TKN_NUMBER)
    __LEXER_TYPE_TO_STRING(TKN_ENDLINE)
    __LEXER_TYPE_TO_STRING(TKN_NEWLINE)
    __LEXER_TYPE_TO_STRING(TKN_OPERATION)
    __LEXER_TYPE_TO_STRING(TKN_COMMA)
    __LEXER_TYPE_TO_STRING(TKN_SPACE)
    __LEXER_TYPE_TO_STRING(TKN_SYMBOL)
    __LEXER_TYPE_TO_STRING(TKN_HASH)
    __LEXER_TYPE_TO_STRING(TKN_COMMENT_MULTILINE)
    __LEXER_TYPE_TO_STRING(TKN_COMMENT_ONENLINE)
    __LEXER_TYPE_TO_STRING(TKN_COMMENT_DOCUMENTATION)
    __LEXER_TYPE_TO_STRING(TKN_EOF)
    __LEXER_TYPE_TO_STRING(TKN_UNKNOWN)
  }
}
int lexer_next_token(struct Lexer *lxr, struct LexerToken *tkn) {
  if (lxr->eof) {
    return 0;
  }

  const char *c = &lxr->string[lxr->cursor];
  size_t start = lxr->cursor;
  size_t len = 0;

LEXER_CHAR_START:
  switch (*c) {
  case 0:
    lxr->eof = 1;
    __LEXER_SET_TOKEN(tkn, TKN_EOF, start, ++len);
    break;
  case '\n':
    __LEXER_SET_TOKEN(tkn, TKN_NEWLINE, start, ++len);
    break;
  case '#':
    __LEXER_SET_TOKEN(tkn, TKN_HASH, start, ++len);
    break;
  case ',':
    __LEXER_SET_TOKEN(tkn, TKN_COMMA, start, ++len);
    break;
  case ' ':
    do {
      c++;
      len++;
    } while (*c && *c == ' ');
    __LEXER_SET_TOKEN(tkn, TKN_SPACE, start, len);
    break;
  case '.':
    if (c[1] >= '0' && c[1] <= '9') {
      len = 1;
      goto LEXER_CHAR_START;
    }

    // if its `...` then its varg operation
    if (c[1] == '.' && c[2] == '.') {
      len = 3;
      __LEXER_SET_TOKEN(tkn, TKN_SYMBOL, start, len);
    } else { // if its a random dot, not followed by another dots or number then
             // its probably dot operation
      __LEXER_SET_TOKEN(tkn, TKN_OPERATION, start, ++len);
    }
    break;
  case ';':
    __LEXER_SET_TOKEN(tkn, TKN_ENDLINE, start, ++len);
    break;
  case '{':
  case '}':
  case '(':
  case ')':
  case '[':
  case ']':
  case '<':
  case '>':
    __LEXER_SET_TOKEN(tkn, TKN_SYMBOL, start, ++len);
    break;
  case '/':
    c++;
    len++;

    if (*c == '/') {
      if (c[1] == '/') { // tripple line is a documentation line
        tkn->type = TKN_COMMENT_DOCUMENTATION;
        len++;
        c++;
      } else {
        tkn->type = TKN_COMMENT_ONENLINE;
      }

      do {
        c++;
        len++;
      } while (*c && *c != '\n');
    } else if (*c == '*') {
      tkn->type = TKN_COMMENT_MULTILINE;
      do {
        c++;
        len++;
      } while (*c && !(*c == '*' && c[1] == '/'));
      c += 2;
      len += 2;
    } else {
      tkn->type = TKN_OPERATION;
    }
    __LEXER_SET_TOKEN(tkn, tkn->type, start, len);
    break;
  case '-':
  case '+':
  case '*':
  case '=':
  case '&':
  case '|':
  case '!':
    len++;
    c++;

    if (*c == '/' || *c == '=' || *c == '>' || *c == '<' || *c == '|') {
      len++;
      c++;
    }
    __LEXER_SET_TOKEN(tkn, TKN_OPERATION, start, len);
    break;
  case '"':
  case '\'':
    do {
      c++;
      len++;

      // checking if the current esacpe char
      if (*c == '\\') {
        len += 2;
        c += 2;
      }
    } while (*c && *c != lxr->string[start]);

    c++;
    len++;
    __LEXER_SET_TOKEN(tkn, TKN_STRING, start, len);
    break;
  case '_':
  case 'a' ... 'z':
  case 'A' ... 'Z':
    do {
      len++;
      c++;
    } while (*c && (isalpha(*c) || *c == '_' || *c == '/'));

    char *name = lexer_get_at_position(lxr, start, len);

    if (lexer_name_is_keyword(name)) {
      __LEXER_SET_TOKEN(tkn, TKN_KEYWORD, start, len);
    } else {
      __LEXER_SET_TOKEN(tkn, TKN_NAME, start, len);
    }
    break;
  case '0' ... '9':
    do {
      len++;
      c++;
    } while (*c && ((*c >= '0' && *c <= '9') || *c == '.'));
    __LEXER_SET_TOKEN(tkn, TKN_NUMBER, start, len);
    break;
  default:
    __LEXER_SET_TOKEN(tkn, TKN_UNKNOWN, start, ++len);
    break;
  }

  lxr->cursor += len;
  return 1;
}

const char *lexer_token_position_string(struct Lexer *lxr,
                                        struct LexerToken *tkn) {
  return lexer_get_at_position(lxr, tkn->start, tkn->len);
}

const char *lexer_token_position_string_r(struct Lexer *lxr,
                                          struct LexerToken *tkn,
                                          char *buffer) {
  return lexer_get_at_position_r(lxr, tkn->start, tkn->len, buffer);
}

static inline const char *lexer_get_at_position(struct Lexer *lxr, size_t start,
                                                size_t len) {
  token_string_buffer = realloc(token_string_buffer, len + 1);
  return lexer_get_at_position_r(lxr, start, len, token_string_buffer);
}

static inline const char *lexer_get_at_position_r(struct Lexer *lxr,
                                                  size_t start, size_t len,
                                                  char *buffer) {
  strncpy(buffer, &(lxr->string[start]), len);
  buffer[len] = 0;
  return buffer;
}

static int lexer_name_is_keyword(const char *name) {
  for (char **keyword = c_keywords; *keyword; keyword++) {
    if (strcmp(name, *keyword) == 0) {
      return 1;
    }
  }
  return 0;
}
