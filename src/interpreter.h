#ifndef TULX_INTERPRETER_H
#define TULX_INTERPRETER_H

#include "stmt.h"
#include "expr.h"
#include "environment.h"
#include "value.h"

/*
 * Interpret a sequence of AST statements within a given lexical environment
 */
void interpret(Stmt** statements, int count, Environment* env);

/*
 * Evaluate a single expression AST node within an environment
 */
Value* evaluate(Expr* expr, Environment* env);

#endif /* TULX_INTERPRETER_H */
