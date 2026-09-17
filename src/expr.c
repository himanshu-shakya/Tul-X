#include "expr.h"
#include "common.h"

/*
 * Centralized Visitor dispatch
 */
void* exprAccept(Expr* expr, ExprVisitor* visitor, void* context) {
    if (expr == NULL) return NULL;

    switch (expr->type) {
        case EXPR_BINARY:
            return visitor->visitBinary(expr, context);
        case EXPR_UNARY:
            return visitor->visitUnary(expr, context);
        case EXPR_LITERAL:
            return visitor->visitLiteral(expr, context);
        case EXPR_GROUPING:
            return visitor->visitGrouping(expr, context);
        case EXPR_VARIABLE:
            return visitor->visitVariable(expr, context);
        case EXPR_ASSIGN:
            return visitor->visitAssign(expr, context);
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

Expr* newVariableExpr(Token name) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    if (!expr) {
        fprintf(stderr, "Out of memory in newVariableExpr.\n");
        exit(EX_SOFTWARE);
    }
    expr->type = EXPR_VARIABLE;
    expr->as.variable.name = name;
    return expr;
}

Expr* newAssignExpr(Token name, Expr* value) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    if (!expr) {
        fprintf(stderr, "Out of memory in newAssignExpr.\n");
        exit(EX_SOFTWARE);
    }
    expr->type = EXPR_ASSIGN;
    expr->as.assign.name = name;
    expr->as.assign.value = value;
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

        case EXPR_VARIABLE:
            /* Token points into source buffer, no heap memory to free */
            break;

        case EXPR_ASSIGN:
            freeExpr(expr->as.assign.value);
            break;
    }

    free(expr);
}
