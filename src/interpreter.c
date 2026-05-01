#include <stdlib.h>
#include <stdio.h>
#include "expr.h"
#include "token.h"

/*
 * Forward declaration of the visitor.
 * Needed because visitor methods call exprAccept recursively.
 */
static ExprVisitor interpreter;

/*
 * visitLiteral
 *
 * Java equivalent:
 *   Object visitLiteralExpr(Literal expr)
 */
static void* evalLiteral(Expr* expr) {
    /* Literal already stores a value */
    return expr->as.literal.value;
}

/*
 * visitGrouping
 *
 * Java equivalent:
 *   Object visitGroupingExpr(Grouping expr)
 */
static void* evalGrouping(Expr* expr) {
    return exprAccept(expr->as.grouping.expression, &interpreter);
}

/*
 * visitUnary
 *
 * Java equivalent:
 *   Object visitUnaryExpr(Unary expr)
 */
static void* evalUnary(Expr* expr) {
    double* right =
        (double*)exprAccept(expr->as.unary.right, &interpreter);

    double* result = malloc(sizeof(double));

    switch (expr->as.unary.operator.type) {
        case TOKEN_MINUS:
            *result = -(*right);
            break;

        default:
            /* unreachable for now */
            *result = 0;
    }

    return result;
}

/*
 * visitBinary
 *
 * Java equivalent:
 *   Object visitBinaryExpr(Binary expr)
 */
static void* evalBinary(Expr* expr) {
    double* left =
        (double*)exprAccept(expr->as.binary.left, &interpreter);

    double* right =
        (double*)exprAccept(expr->as.binary.right, &interpreter);

    double* result = malloc(sizeof(double));

    switch (expr->as.binary.operator.type) {
        case TOKEN_PLUS:
            *result = *left + *right;
            break;

        case TOKEN_MINUS:
            *result = *left - *right;
            break;

        case TOKEN_STAR:
            *result = *left * *right;
            break;

        case TOKEN_SLASH:
            *result = *left / *right;
            break;

        default:
            /* unreachable for now */
            *result = 0;
    }

    return result;
}

/*
 * The actual Visitor instance.
 *
 * Java equivalent:
 *   class Interpreter implements Expr.Visitor<Object>
 */
static ExprVisitor interpreter = {
    .visitBinary   = evalBinary,
    .visitUnary    = evalUnary,
    .visitLiteral  = evalLiteral,
    .visitGrouping = evalGrouping
};
