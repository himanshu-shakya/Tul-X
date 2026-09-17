#ifndef TULX_ENVIRONMENT_H
#define TULX_ENVIRONMENT_H

#include "common.h"
#include "value.h"
#include "token.h"

typedef struct Entry {
    char* key;
    Value value;
    struct Entry* next;
} Entry;

typedef struct Environment {
    struct Environment* enclosing;
    Entry** buckets;
    int capacity;
    int count;
} Environment;

/*
 * Create a new lexical environment, optionally chained to an enclosing parent scope
 */
Environment* newEnvironment(Environment* enclosing);

/*
 * Deallocate an environment and all its local variable bindings
 */
void freeEnvironment(Environment* env);

/*
 * Define a variable in the innermost (current) scope
 */
void envDefine(Environment* env, const char* name, int length, Value value);

/*
 * Look up a variable by token in current scope or parent chain
 */
bool envGet(Environment* env, Token name, Value* outValue);

/*
 * Assign a new value to an existing variable in the closest matching scope
 */
bool envAssign(Environment* env, Token name, Value value);

#endif /* TULX_ENVIRONMENT_H */
