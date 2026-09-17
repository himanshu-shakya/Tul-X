#ifndef TULX_PARSER_H
#define TULX_PARSER_H

#include "token.h"
#include "expr.h"
#include "stmt.h"

typedef struct {
    Token* tokens;
    int count;
    int current;
} Parser;

Parser makeParser(Token* tokens, int count);
Stmt** parse(Parser* parser, int* outCount);
Expr* parseExpressionOnly(Parser* parser);

#endif /* TULX_PARSER_H */
