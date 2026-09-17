#include "interpreter.h"
#include "error.h"
#include "common.h"

/* Forward declarations */
static ExprVisitor exprVisitor;
static StmtVisitor stmtVisitor;

/* ============================================================================
 * Expression Evaluation
 * ============================================================================ */

static void* evalLiteral(Expr* expr, void* context) {
    (void)context;
    Value* result = (Value*)malloc(sizeof(Value));
    if (!result) return NULL;
    *result = copyValue(expr->as.literal.value);
    return result;
}

static void* evalGrouping(Expr* expr, void* context) {
    return exprAccept(expr->as.grouping.expression, &exprVisitor, context);
}

static void* evalVariable(Expr* expr, void* context) {
    Environment* env = (Environment*)context;
    Value val;
    if (!envGet(env, expr->as.variable.name, &val)) {
        runtimeError(expr->as.variable.name, "Undefined variable '%.*s'.",
                     expr->as.variable.name.length, expr->as.variable.name.start);
        return NULL;
    }

    Value* result = (Value*)malloc(sizeof(Value));
    if (!result) {
        freeValue(val);
        return NULL;
    }
    *result = val; /* envGet already returned a copied Value */
    return result;
}

static void* evalAssign(Expr* expr, void* context) {
    Environment* env = (Environment*)context;
    Value* val = (Value*)exprAccept(expr->as.assign.value, &exprVisitor, context);
    if (val == NULL) return NULL;

    if (!envAssign(env, expr->as.assign.name, *val)) {
        runtimeError(expr->as.assign.name, "Undefined variable '%.*s'.",
                     expr->as.assign.name.length, expr->as.assign.name.start);
        freeValue(*val);
        free(val);
        return NULL;
    }

    return val;
}

static void* evalUnary(Expr* expr, void* context) {
    Value* right = (Value*)exprAccept(expr->as.unary.right, &exprVisitor, context);
    if (!right) return NULL;

    Value* result = (Value*)malloc(sizeof(Value));
    if (!result) {
        freeValue(*right);
        free(right);
        return NULL;
    }

    switch (expr->as.unary.operator.type) {
        case TOKEN_MINUS:
            if (!IS_NUMBER(*right)) {
                runtimeError(expr->as.unary.operator, "Operand must be a number.");
                freeValue(*right);
                free(right);
                free(result);
                return NULL;
            }
            *result = NUMBER_VAL(-AS_NUMBER(*right));
            break;

        case TOKEN_BANG: {
            /* Truthiness: null and false are falsey, everything else is truthy */
            bool isFalsey = IS_NULL(*right) || (IS_BOOL(*right) && !AS_BOOL(*right));
            *result = BOOL_VAL(isFalsey);
            break;
        }

        default:
            *result = NULL_VAL();
            break;
    }

    freeValue(*right);
    free(right);
    return result;
}

static void* evalBinary(Expr* expr, void* context) {
    Value* left = (Value*)exprAccept(expr->as.binary.left, &exprVisitor, context);
    if (!left) return NULL;

    Value* right = (Value*)exprAccept(expr->as.binary.right, &exprVisitor, context);
    if (!right) {
        freeValue(*left);
        free(left);
        return NULL;
    }

    Value* result = (Value*)malloc(sizeof(Value));
    if (!result) {
        freeValue(*left);
        free(left);
        freeValue(*right);
        free(right);
        return NULL;
    }

    switch (expr->as.binary.operator.type) {
        case TOKEN_PLUS:
            if (IS_NUMBER(*left) && IS_NUMBER(*right)) {
                *result = NUMBER_VAL(AS_NUMBER(*left) + AS_NUMBER(*right));
            } else if (IS_STRING(*left) || IS_STRING(*right)) {
                /* String concatenation */
                char leftBuf[64];
                const char* leftStr;
                if (IS_STRING(*left)) {
                    leftStr = AS_STRING(*left);
                } else if (IS_NUMBER(*left)) {
                    snprintf(leftBuf, sizeof(leftBuf), "%g", AS_NUMBER(*left));
                    leftStr = leftBuf;
                } else if (IS_BOOL(*left)) {
                    leftStr = AS_BOOL(*left) ? "true" : "false";
                } else {
                    leftStr = "null";
                }

                char rightBuf[64];
                const char* rightStr;
                if (IS_STRING(*right)) {
                    rightStr = AS_STRING(*right);
                } else if (IS_NUMBER(*right)) {
                    snprintf(rightBuf, sizeof(rightBuf), "%g", AS_NUMBER(*right));
                    rightStr = rightBuf;
                } else if (IS_BOOL(*right)) {
                    rightStr = AS_BOOL(*right) ? "true" : "false";
                } else {
                    rightStr = "null";
                }

                size_t len1 = strlen(leftStr);
                size_t len2 = strlen(rightStr);
                char* combined = (char*)malloc(len1 + len2 + 1);
                if (!combined) {
                    fprintf(stderr, "Out of memory in string concatenation.\n");
                    exit(EX_SOFTWARE);
                }
                memcpy(combined, leftStr, len1);
                memcpy(combined + len1, rightStr, len2);
                combined[len1 + len2] = '\0';

                *result = STRING_VAL(combined);
            } else {
                runtimeError(expr->as.binary.operator, "Operands must be two numbers or strings.");
                freeValue(*left);
                free(left);
                freeValue(*right);
                free(right);
                free(result);
                return NULL;
            }
            break;

        case TOKEN_MINUS:
            if (!IS_NUMBER(*left) || !IS_NUMBER(*right)) {
                runtimeError(expr->as.binary.operator, "Operands must be numbers.");
                freeValue(*left); free(left); freeValue(*right); free(right); free(result);
                return NULL;
            }
            *result = NUMBER_VAL(AS_NUMBER(*left) - AS_NUMBER(*right));
            break;

        case TOKEN_STAR:
            if (!IS_NUMBER(*left) || !IS_NUMBER(*right)) {
                runtimeError(expr->as.binary.operator, "Operands must be numbers.");
                freeValue(*left); free(left); freeValue(*right); free(right); free(result);
                return NULL;
            }
            *result = NUMBER_VAL(AS_NUMBER(*left) * AS_NUMBER(*right));
            break;

        case TOKEN_SLASH:
            if (!IS_NUMBER(*left) || !IS_NUMBER(*right)) {
                runtimeError(expr->as.binary.operator, "Operands must be numbers.");
                freeValue(*left); free(left); freeValue(*right); free(right); free(result);
                return NULL;
            }
            *result = NUMBER_VAL(AS_NUMBER(*left) / AS_NUMBER(*right));
            break;

        case TOKEN_GREATER:
            if (!IS_NUMBER(*left) || !IS_NUMBER(*right)) {
                runtimeError(expr->as.binary.operator, "Operands must be numbers.");
                freeValue(*left); free(left); freeValue(*right); free(right); free(result);
                return NULL;
            }
            *result = BOOL_VAL(AS_NUMBER(*left) > AS_NUMBER(*right));
            break;

        case TOKEN_GREATER_EQUAL:
            if (!IS_NUMBER(*left) || !IS_NUMBER(*right)) {
                runtimeError(expr->as.binary.operator, "Operands must be numbers.");
                freeValue(*left); free(left); freeValue(*right); free(right); free(result);
                return NULL;
            }
            *result = BOOL_VAL(AS_NUMBER(*left) >= AS_NUMBER(*right));
            break;

        case TOKEN_LESS:
            if (!IS_NUMBER(*left) || !IS_NUMBER(*right)) {
                runtimeError(expr->as.binary.operator, "Operands must be numbers.");
                freeValue(*left); free(left); freeValue(*right); free(right); free(result);
                return NULL;
            }
            *result = BOOL_VAL(AS_NUMBER(*left) < AS_NUMBER(*right));
            break;

        case TOKEN_LESS_EQUAL:
            if (!IS_NUMBER(*left) || !IS_NUMBER(*right)) {
                runtimeError(expr->as.binary.operator, "Operands must be numbers.");
                freeValue(*left); free(left); freeValue(*right); free(right); free(result);
                return NULL;
            }
            *result = BOOL_VAL(AS_NUMBER(*left) <= AS_NUMBER(*right));
            break;

        case TOKEN_EQUAL_EQUAL:
            *result = BOOL_VAL(valuesEqual(*left, *right));
            break;

        case TOKEN_BANG_EQUAL:
            *result = BOOL_VAL(!valuesEqual(*left, *right));
            break;

        default:
            *result = NULL_VAL();
            break;
    }

    freeValue(*left);
    free(left);
    freeValue(*right);
    free(right);
    return result;
}

static ExprVisitor exprVisitor = {
    .visitBinary   = evalBinary,
    .visitUnary    = evalUnary,
    .visitLiteral  = evalLiteral,
    .visitGrouping = evalGrouping,
    .visitVariable = evalVariable,
    .visitAssign   = evalAssign
};

/* ============================================================================
 * Statement Execution
 * ============================================================================ */

static void execExpr(Stmt* stmt, void* context) {
    Value* val = (Value*)exprAccept(stmt->as.expr.expression, &exprVisitor, context);
    if (val != NULL) {
        freeValue(*val);
        free(val);
    }
}

static void execPrint(Stmt* stmt, void* context) {
    Value* val = (Value*)exprAccept(stmt->as.print.expression, &exprVisitor, context);
    if (val != NULL) {
        printValue(*val);
        printf("\n");
        freeValue(*val);
        free(val);
    }
}

static void execVar(Stmt* stmt, void* context) {
    Environment* env = (Environment*)context;
    Value val = NULL_VAL();

    if (stmt->as.var.initializer != NULL) {
        Value* initVal = (Value*)exprAccept(stmt->as.var.initializer, &exprVisitor, context);
        if (initVal != NULL) {
            val = *initVal;
            free(initVal);
        } else {
            return; /* Runtime error during expression evaluation */
        }
    }

    envDefine(env, stmt->as.var.name.start, stmt->as.var.name.length, val);
    freeValue(val);
}

static void execBlock(Stmt* stmt, void* context) {
    Environment* enclosing = (Environment*)context;
    Environment* blockEnv = newEnvironment(enclosing);

    for (int i = 0; i < stmt->as.block.count; i++) {
        stmtAccept(stmt->as.block.statements[i], &stmtVisitor, blockEnv);
        if (hadRuntimeError) break;
    }

    freeEnvironment(blockEnv);
}

static StmtVisitor stmtVisitor = {
    .visitExpr  = execExpr,
    .visitPrint = execPrint,
    .visitVar   = execVar,
    .visitBlock = execBlock
};

/* ============================================================================
 * Public Interpreter API
 * ============================================================================ */

Value* evaluate(Expr* expr, Environment* env) {
    if (expr == NULL) return NULL;
    return (Value*)exprAccept(expr, &exprVisitor, env);
}

void interpret(Stmt** statements, int count, Environment* env) {
    if (statements == NULL || env == NULL) return;

    for (int i = 0; i < count; i++) {
        if (statements[i] != NULL) {
            stmtAccept(statements[i], &stmtVisitor, env);
            if (hadRuntimeError) break;
        }
    }
}
