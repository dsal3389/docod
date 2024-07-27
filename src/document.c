
#include "document.h"
#include "lexer.h"

void doc_init(struct DocContext *ctx, struct Lexer *lxr) { ctx->lxr = lxr; }
