#ifndef TOKEN_H
#define TOKEN_H

#include "tokentype.h"

typedef struct {
    TokenType type;
    const char* start;
    int length;
    void* literal;
    int line;
} Token;

Token makeToken(
    TokenType type,
    const char* start,
    int length,
    void* literal,
    int line
);

void printToken(Token token);

#endif
