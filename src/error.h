#ifndef TULX_ERROR_H
#define TULX_ERROR_H

#include <stdbool.h>

extern bool hadError;
extern bool hadRuntimeError;

void error(int line, const char* message);
void report(int line, const char* where, const char* message);

#endif /* TULX_ERROR_H */