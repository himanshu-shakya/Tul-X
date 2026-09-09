#ifndef TULX_PARSER_H
#define TULX_PARSER_H

#include "token.h"
#include "expr.h"

typedef struct {
    Token* tokens;
    int count;
    int current;
} Parser;

Parser makeParser(Token* tokens, int count);
Expr* parse(Parser* parser);

#endif /* TULX_PARSER_H */
