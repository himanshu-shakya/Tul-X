#include "expr.h"

/*
 * Centralized Visitor dispatch
 */
void* exprAccept(Expr* expr, ExprVisitor* visitor) {
    if (expr == NULL) return NULL;

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

    return NULL;
}

/*
 * Factory functions (constructors in C)
 */

Expr* newBinaryExpr(Expr* left, Token operator, Expr* right) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    if (!expr) {
        fprintf(stderr, "Out of memory in newBinaryExpr.\n");
        exit(EX_SOFTWARE);
    }
    expr->type = EXPR_BINARY;
    expr->as.binary.left = left;
    expr->as.binary.operator = operator;
    expr->as.binary.right = right;
    return expr;
}

Expr* newUnaryExpr(Token operator, Expr* right) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    if (!expr) {
        fprintf(stderr, "Out of memory in newUnaryExpr.\n");
        exit(EX_SOFTWARE);
    }
    expr->type = EXPR_UNARY;
    expr->as.unary.operator = operator;
    expr->as.unary.right = right;
    return expr;
}

Expr* newLiteralExpr(Value value) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    if (!expr) {
        fprintf(stderr, "Out of memory in newLiteralExpr.\n");
        exit(EX_SOFTWARE);
    }
    expr->type = EXPR_LITERAL;
    expr->as.literal.value = value;
    return expr;
}

Expr* newGroupingExpr(Expr* expression) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    if (!expr) {
        fprintf(stderr, "Out of memory in newGroupingExpr.\n");
        exit(EX_SOFTWARE);
    }
    expr->type = EXPR_GROUPING;
    expr->as.grouping.expression = expression;
    return expr;
}

/*
 * Destructor: Safely and recursively deallocates AST nodes
 */
void freeExpr(Expr* expr) {
    if (expr == NULL) return;

    switch (expr->type) {
        case EXPR_BINARY:
            freeExpr(expr->as.binary.left);
            freeExpr(expr->as.binary.right);
            break;

        case EXPR_UNARY:
            freeExpr(expr->as.unary.right);
            break;

        case EXPR_GROUPING:
            freeExpr(expr->as.grouping.expression);
            break;

        case EXPR_LITERAL:
            freeValue(expr->as.literal.value);
            break;
    }

    free(expr);
}
