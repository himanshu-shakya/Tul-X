#include <stdio.h>
#include "token.h"

Token makeToken(
    TokenType type,
    const char* start,
    int length,
    void* literal,
    int line
) {
    Token token;
    token.type = type;
    token.start = start;
    token.length = length;
    token.literal = literal;
    token.line = line;
    return token;
}

void printToken(Token token) {
    printf("%-15d '%.*s'\n",
           token.type,
           token.length,
           token.start);
}
