#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"

typedef struct {
    const char* source;
    int length;

    int start;
    int current;
    int line;

    Token* tokens;
    int count;
    int capacity;
} Scanner;

Scanner makeScanner(const char* source);
void scanTokens(Scanner* scanner);

#endif
