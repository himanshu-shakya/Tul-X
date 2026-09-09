#include "common.h"
#include "expr.h"
#include "value.h"

/*
 * Forward declaration of the visitor
 */
static ExprVisitor interpreter;

static void* evalLiteral(Expr* expr) {
    Value* result = (Value*)malloc(sizeof(Value));
    if (!result) return NULL;
    *result = expr->as.literal.value;
    return result;
}

static void* evalGrouping(Expr* expr) {
    return exprAccept(expr->as.grouping.expression, &interpreter);
}

static void* evalUnary(Expr* expr) {
    Value* right = (Value*)exprAccept(expr->as.unary.right, &interpreter);
    if (!right) return NULL;

    Value* result = (Value*)malloc(sizeof(Value));
    if (!result) {
        free(right);
        return NULL;
    }

    switch (expr->as.unary.operator.type) {
        case TOKEN_MINUS:
            if (IS_NUMBER(*right)) {
                *result = NUMBER_VAL(-AS_NUMBER(*right));
            } else {
                *result = NULL_VAL();
            }
            break;

        case TOKEN_BANG:
            /* truthiness: null and false are falsey, everything else is truthy */
            if (IS_NULL(*right) || (IS_BOOL(*right) && !AS_BOOL(*right))) {
                *result = BOOL_VAL(true);
            } else {
                *result = BOOL_VAL(false);
            }
            break;

        default:
            *result = NULL_VAL();
            break;
    }

    free(right);
    return result;
}

static void* evalBinary(Expr* expr) {
    Value* left = (Value*)exprAccept(expr->as.binary.left, &interpreter);
    Value* right = (Value*)exprAccept(expr->as.binary.right, &interpreter);

    if (!left || !right) {
        if (left) free(left);
        if (right) free(right);
        return NULL;
    }

    Value* result = (Value*)malloc(sizeof(Value));
    if (!result) {
        free(left);
        free(right);
        return NULL;
    }

    switch (expr->as.binary.operator.type) {
        case TOKEN_PLUS:
            if (IS_NUMBER(*left) && IS_NUMBER(*right)) {
                *result = NUMBER_VAL(AS_NUMBER(*left) + AS_NUMBER(*right));
            } else {
                *result = NULL_VAL();
            }
            break;

        case TOKEN_MINUS:
            if (IS_NUMBER(*left) && IS_NUMBER(*right)) {
                *result = NUMBER_VAL(AS_NUMBER(*left) - AS_NUMBER(*right));
            } else {
                *result = NULL_VAL();
            }
            break;

        case TOKEN_STAR:
            if (IS_NUMBER(*left) && IS_NUMBER(*right)) {
                *result = NUMBER_VAL(AS_NUMBER(*left) * AS_NUMBER(*right));
            } else {
                *result = NULL_VAL();
            }
            break;

        case TOKEN_SLASH:
            if (IS_NUMBER(*left) && IS_NUMBER(*right)) {
                *result = NUMBER_VAL(AS_NUMBER(*left) / AS_NUMBER(*right));
            } else {
                *result = NULL_VAL();
            }
            break;

        case TOKEN_GREATER:
            if (IS_NUMBER(*left) && IS_NUMBER(*right)) {
                *result = BOOL_VAL(AS_NUMBER(*left) > AS_NUMBER(*right));
            } else {
                *result = BOOL_VAL(false);
            }
            break;

        case TOKEN_GREATER_EQUAL:
            if (IS_NUMBER(*left) && IS_NUMBER(*right)) {
                *result = BOOL_VAL(AS_NUMBER(*left) >= AS_NUMBER(*right));
            } else {
                *result = BOOL_VAL(false);
            }
            break;

        case TOKEN_LESS:
            if (IS_NUMBER(*left) && IS_NUMBER(*right)) {
                *result = BOOL_VAL(AS_NUMBER(*left) < AS_NUMBER(*right));
            } else {
                *result = BOOL_VAL(false);
            }
            break;

        case TOKEN_LESS_EQUAL:
            if (IS_NUMBER(*left) && IS_NUMBER(*right)) {
                *result = BOOL_VAL(AS_NUMBER(*left) <= AS_NUMBER(*right));
            } else {
                *result = BOOL_VAL(false);
            }
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

    free(left);
    free(right);
    return result;
}

static ExprVisitor interpreter = {
    .visitBinary   = evalBinary,
    .visitUnary    = evalUnary,
    .visitLiteral  = evalLiteral,
    .visitGrouping = evalGrouping
};

Value* interpret(Expr* expr) {
    if (expr == NULL) return NULL;
    return (Value*)exprAccept(expr, &interpreter);
}
