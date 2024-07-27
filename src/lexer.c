#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

static char *lxr__tokn_cpy_string(char *, const char *, size_t);

void lexer_init(struct Lexer *lxr, const char *string) {
  lxr->string = string;
  lxr->pos = string;
  lxr->eof = 0;
}

void lexer_token_init(struct LexerToken *tkn) {
  tkn->type = TKN_UNKNOWN;
  tkn->string = NULL;
  tkn->start = 0;
  tkn->end = 0;
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
    return -1;
  }

  const char *c = lxr->pos;
  const char *start = c;
  size_t cpy_len = 0;

LEXER_CHAR_START:
  switch (*c) {
  case 0:
    lxr->eof = 1;
    tkn->type = TKN_EOF;
    tkn->string = lxr__tokn_cpy_string(tkn->string, c, 1);
    break;
  case '\n':
    tkn->type = TKN_NEWLINE;
    tkn->string = lxr__tokn_cpy_string(tkn->string, c++, 1);
    break;
  case '#':
    tkn->type = TKN_HASH;
    tkn->string = lxr__tokn_cpy_string(tkn->string, c++, 1);
    break;
  case ',':
    tkn->type = TKN_COMMA;
    tkn->string = lxr__tokn_cpy_string(tkn->string, c++, 1);
    break;
  case ' ':
    do {
      c++;
    } while (*c && *c == ' ');
    tkn->type = TKN_SPACE;
    tkn->string = lxr__tokn_cpy_string(tkn->string, start, 1);
    break;
  case '.':
    if (c[1] >= '0' && c[1] <= '9') {
      cpy_len = 1;
      goto LEXER_CHAR_START;
    }

    // if its `...` then its varg operation
    if (c[1] == '.' && c[2] == '.') {
      tkn->type = TKN_SYMBOL;
      tkn->string = lxr__tokn_cpy_string(tkn->string, c, 3);
      c += 3;
    } else { // if its a random dot, not followed by another dots or number then
             // its probably dot operation
      tkn->type = TKN_OPERATION;
      tkn->string = lxr__tokn_cpy_string(tkn->string, c++, 1);
    }
    break;
  case ';':
    tkn->type = TKN_ENDLINE;
    tkn->string = lxr__tokn_cpy_string(tkn->string, c++, 1);
    break;
  case '{':
  case '}':
  case '(':
  case ')':
  case '[':
  case ']':
  case '<':
  case '>':
    tkn->type = TKN_SYMBOL;
    tkn->string = lxr__tokn_cpy_string(tkn->string, c++, 1);
    break;
  case '/':
    c++;
    cpy_len++;

    if (*c == '/') {
      if (c[1] == '/') { // tripple line is a documentation line
        tkn->type = TKN_COMMENT_DOCUMENTATION;
        cpy_len++;
        c++;
      } else {
        tkn->type = TKN_COMMENT_ONENLINE;
      }

      do {
        c++;
        cpy_len++;
      } while (*c && *c != '\n');
    } else if (*c == '*') {
      tkn->type = TKN_COMMENT_MULTILINE;
      do {
        c++;
        cpy_len++;
      } while (*c && !(*c == '*' && c[1] == '/'));
      c += 2;
      cpy_len += 2;
    } else {
      tkn->type = TKN_OPERATION;
    }
    tkn->string = lxr__tokn_cpy_string(tkn->string, start, cpy_len);
    break;
  case '-':
  case '+':
  case '*':
  case '=':
  case '&':
  case '|':
  case '!':
    cpy_len++;
    c++;

    if (*c == '/' || *c == '=' || *c == '>' || *c == '<' || *c == '|') {
      cpy_len++;
      c++;
    }
    tkn->type = TKN_OPERATION;
    tkn->string = lxr__tokn_cpy_string(tkn->string, start, cpy_len);
    break;
  case '"':
  case '\'':
    do {
      c++;
      cpy_len++;

      // checking if the current esacpe char
      if (*c == '\\') {
        cpy_len += 2;
        c += 2;
      }
    } while (*c && *c != *start);

    c++;
    cpy_len++;

    tkn->type = TKN_STRING;
    tkn->string = lxr__tokn_cpy_string(tkn->string, start, cpy_len);
    break;
  case '_':
  case 'a' ... 'z':
  case 'A' ... 'Z':
    do {
      cpy_len++;
      c++;
    } while (*c && (isalpha(*c) || *c == '_' || *c == '/'));

    tkn->type = TKN_NAME;
    tkn->string = lxr__tokn_cpy_string(tkn->string, start, cpy_len);
    break;
  case '0' ... '9':
    do {
      cpy_len++;
      c++;
    } while (*c && ((*c >= '0' && *c <= '9') || *c == '.'));
    tkn->type = TKN_NUMBER;
    tkn->string = lxr__tokn_cpy_string(tkn->string, start, cpy_len);
    break;
  default:
    c++;
    tkn->type = TKN_UNKNOWN;
    tkn->string = lxr__tokn_cpy_string(tkn->string, start, 1);
    break;
  }

  lxr->pos = c;
  return 0;
}

static char *lxr__tokn_cpy_string(char *buffer, const char *string,
                                  size_t length) {
  char *string_buffer = realloc(buffer, length + 1);
  string_buffer[length] = 0;

  strncpy(string_buffer, string, length);
  return string_buffer;
}
