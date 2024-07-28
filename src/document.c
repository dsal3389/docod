
#include "document.h"
#include "dll.h"
#include "lexer.h"

void doc_init(struct DocumentCtx *ctx, const char *path, struct Lexer *lxr) {
  ctx->path = path;
  ctx->lxr = lxr;
  dll_init(&ctx->items);
}

void doc_start(struct DocumentCtx *ctx) {
  struct LexerToken tkn;

  while (lexer_next_token(ctx->lxr, &tkn)) {
    switch (tkn.type) {
    case TKN_KEYWORD:
      break;
    case TKN_COMMENT_DOCUMENTATION:
      break;
    default:
      break;
    }
  }
}
