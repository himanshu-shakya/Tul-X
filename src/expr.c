#include <stdlib.h>
#include "expr.h"

/*
 * Centralized Visitor dispatch
 *
 * This is the equivalent of:
 *
 *   <R> R accept(Visitor<R> visitor)
 *
 * in Java.
 *
 * This is the ONLY place where we switch on ExprType.
 */
void* exprAccept(Expr* expr, ExprVisitor* visitor) {
    switch (expr->type) {
        case EXPR_BINARY:
            return visitor->visitBinary(expr);

        case EXPR_UNARY:
            return visitor->visitUnary(expr);

        case EXPR_LITERAL:
            return visitor->visitLiteral(expr);

        case EXPR_GROUPING:
            return visitor->visitGrouping(expr);
    }

    /* Unreachable, but keeps the compiler happy */
    return NULL;
}

/*
 * Factory functions (constructors in C)
 */

Expr* newBinaryExpr(Expr* left, Token operator, Expr* right) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    expr->type = EXPR_BINARY;
    expr->as.binary.left = left;
    expr->as.binary.operator = operator;
    expr->as.binary.right = right;
    return expr;
}

Expr* newUnaryExpr(Token operator, Expr* right) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    expr->type = EXPR_UNARY;
    expr->as.unary.operator = operator;
    expr->as.unary.right = right;
    return expr;
}

Expr* newLiteralExpr(void* value) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->as.literal.value = value;
    return expr;
}

Expr* newGroupingExpr(Expr* expression) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    expr->type = EXPR_GROUPING;
    expr->as.grouping.expression = expression;
    return expr;
}
