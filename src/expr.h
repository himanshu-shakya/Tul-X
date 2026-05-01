#ifndef TULX_EXPR_H
#define TULX_EXPR_H

#include "token.h"

/*
 * Forward declarations
 */
typedef struct Expr Expr;
typedef struct ExprVisitor ExprVisitor;

/*
 * Replaces Java subclasses (Binary, Unary, Literal, Grouping)
 */
typedef enum {
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_LITERAL,
    EXPR_GROUPING
} ExprType;

/*
 * Base AST node (tagged union)
 *
 * This is the equivalent of:
 *   abstract class Expr { ... }
 */
struct Expr {
    ExprType type;   /* which kind of expression this is */

    union {
        /* Binary expression: left operator right */
        struct {
            Expr* left;
            Token operator;
            Expr* right;
        } binary;

        /* Unary expression: operator right */
        struct {
            Token operator;
            Expr* right;
        } unary;

        /* Literal value: number, string, true, false, nil */
        struct {
            void* value;
        } literal;

        /* Grouping: ( expression ) */
        struct {
            Expr* expression;
        } grouping;
    } as;
};

/*
 * Visitor interface
 *
 * C equivalent of:
 *
 * interface Visitor<R> {
 *   R visitBinaryExpr(Binary expr);
 *   R visitUnaryExpr(Unary expr);
 *   R visitLiteralExpr(Literal expr);
 *   R visitGroupingExpr(Grouping expr);
 * }
 *
 * Return type is void* to simulate generics.
 */
struct ExprVisitor {
    void* (*visitBinary)(Expr* expr);
    void* (*visitUnary)(Expr* expr);
    void* (*visitLiteral)(Expr* expr);
    void* (*visitGrouping)(Expr* expr);
};

/*
 * accept() equivalent
 *
 * Centralized double-dispatch point.
 * This is the ONLY switch on ExprType you need.
 */
void* exprAccept(Expr* expr, ExprVisitor* visitor);

/*
 * Factory functions (constructors in C)
 */
Expr* newBinaryExpr(Expr* left, Token operator, Expr* right);
Expr* newUnaryExpr(Token operator, Expr* right);
Expr* newLiteralExpr(void* value);
Expr* newGroupingExpr(Expr* expression);

#endif /* TULX_EXPR_H */
