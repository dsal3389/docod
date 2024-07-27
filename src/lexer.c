#include <ctype.h>
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

static char *lxr__tokn_cpy_string(char *, const char *, size_t);

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
    __LEXER_SET_TOKEN(tkn, TKN_NAME, start, len);
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
  token_string_buffer = realloc(token_string_buffer, tkn->len + 1);
  lexer_token_position_string_r(lxr, tkn, token_string_buffer);
  return token_string_buffer;
}

void lexer_token_position_string_r(struct Lexer *lxr, struct LexerToken *tkn,
                                   char *buffer) {
  strncpy(buffer, &(lxr->string[tkn->start]), tkn->len);
  buffer[tkn->len] = 0;
}

static char *lxr__tokn_cpy_string(char *buffer, const char *string,
                                  size_t length) {
  char *string_buffer = realloc(buffer, length + 1);
  string_buffer[length] = 0;

  strncpy(string_buffer, string, length);
  return string_buffer;
}
