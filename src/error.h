#ifndef TULX_ERROR_H
#define TULX_ERROR_H

#include <stdbool.h>
#include "token.h"

extern bool hadError;
extern bool hadRuntimeError;

void error(int line, const char* message);
void report(int line, const char* where, const char* message);
void runtimeError(Token token, const char* format, ...);

#endif /* TULX_ERROR_H */