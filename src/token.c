#include "token.h"
#include <stdio.h>

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

const char* tokenTypeToString(TokenType type) {
    switch (type) {
        /* Single-character tokens */
        case TOKEN_LEFT_PAREN:    return "TOKEN_LEFT_PAREN";
        case TOKEN_RIGHT_PAREN:   return "TOKEN_RIGHT_PAREN";
        case TOKEN_LEFT_BRACE:    return "TOKEN_LEFT_BRACE";
        case TOKEN_RIGHT_BRACE:   return "TOKEN_RIGHT_BRACE";
        case TOKEN_COMMA:         return "TOKEN_COMMA";
        case TOKEN_DOT:           return "TOKEN_DOT";
        case TOKEN_MINUS:         return "TOKEN_MINUS";
        case TOKEN_PLUS:          return "TOKEN_PLUS";
        case TOKEN_SEMICOLON:     return "TOKEN_SEMICOLON";
        case TOKEN_SLASH:         return "TOKEN_SLASH";
        case TOKEN_STAR:          return "TOKEN_STAR";

        /* One or two character tokens */
        case TOKEN_BANG:          return "TOKEN_BANG";
        case TOKEN_BANG_EQUAL:    return "TOKEN_BANG_EQUAL";
        case TOKEN_EQUAL:         return "TOKEN_EQUAL";
        case TOKEN_EQUAL_EQUAL:   return "TOKEN_EQUAL_EQUAL";
        case TOKEN_GREATER:       return "TOKEN_GREATER";
        case TOKEN_GREATER_EQUAL: return "TOKEN_GREATER_EQUAL";
        case TOKEN_LESS:          return "TOKEN_LESS";
        case TOKEN_LESS_EQUAL:    return "TOKEN_LESS_EQUAL";

        /* Literals */
        case TOKEN_IDENTIFIER:    return "TOKEN_IDENTIFIER";
        case TOKEN_STRING:        return "TOKEN_STRING";
        case TOKEN_NUMBER:        return "TOKEN_NUMBER";

        /* Keywords */
        case TOKEN_AND:           return "TOKEN_AND";
        case TOKEN_CLASS:         return "TOKEN_CLASS";
        case TOKEN_ELSE:          return "TOKEN_ELSE";
        case TOKEN_FALSE:         return "TOKEN_FALSE";
        case TOKEN_FUN:           return "TOKEN_FUN";
        case TOKEN_FOR:           return "TOKEN_FOR";
        case TOKEN_IF:            return "TOKEN_IF";
        case TOKEN_NULL:          return "TOKEN_NULL";
        case TOKEN_OR:            return "TOKEN_OR";
        case TOKEN_PRINT:         return "TOKEN_PRINT";
        case TOKEN_RETURN:        return "TOKEN_RETURN";
        case TOKEN_SUPER:         return "TOKEN_SUPER";
        case TOKEN_THIS:          return "TOKEN_THIS";
        case TOKEN_TRUE:          return "TOKEN_TRUE";
        case TOKEN_VAR:           return "TOKEN_VAR";
        case TOKEN_WHILE:         return "TOKEN_WHILE";

        /* Special */
        case TOKEN_EOF:           return "TOKEN_EOF";
    }

    return "TOKEN_UNKNOWN";
}

void printToken(Token token) {
    if (token.type == TOKEN_EOF) {
        printf("[line %2d] %-20s (EOF)\n", token.line, tokenTypeToString(token.type));
    } else if (token.type == TOKEN_NUMBER && token.literal != NULL) {
        printf("[line %2d] %-20s '%.*s' (value: %g)\n",
               token.line,
               tokenTypeToString(token.type),
               token.length,
               token.start,
               *(double*)token.literal);
    } else if (token.type == TOKEN_STRING && token.literal != NULL) {
        printf("[line %2d] %-20s '%.*s' (value: \"%s\")\n",
               token.line,
               tokenTypeToString(token.type),
               token.length,
               token.start,
               (char*)token.literal);
    } else {
        printf("[line %2d] %-20s '%.*s'\n",
               token.line,
               tokenTypeToString(token.type),
               token.length,
               token.start);
    }
}
