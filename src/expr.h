#ifndef TULX_EXPR_H
#define TULX_EXPR_H

#include "token.h"
#include "value.h"

/*
 * Forward declarations
 */
typedef struct Expr Expr;
typedef struct ExprVisitor ExprVisitor;

/*
 * Expression types
 */
typedef enum {
    EXPR_BINARY,
    EXPR_LOGICAL,
    EXPR_UNARY,
    EXPR_LITERAL,
    EXPR_GROUPING,
    EXPR_VARIABLE,
    EXPR_ASSIGN
} ExprType;

/*
 * Base AST node (tagged union)
 */
struct Expr {
    ExprType type;

    union {
        /* Binary expression: left operator right */
        struct {
            Expr* left;
            Token operator;
            Expr* right;
        } binary;

        /* Logical expression: left operator right (short-circuiting and / or) */
        struct {
            Expr* left;
            Token operator;
            Expr* right;
        } logical;

        /* Unary expression: operator right */
        struct {
            Token operator;
            Expr* right;
        } unary;

        /* Literal value: number, string, true, false, null */
        struct {
            Value value;
        } literal;

        /* Grouping: ( expression ) */
        struct {
            Expr* expression;
        } grouping;

        /* Variable access: name */
        struct {
            Token name;
        } variable;

        /* Variable assignment: name = value */
        struct {
            Token name;
            Expr* value;
        } assign;
    } as;
};

/*
 * Visitor interface
 */
struct ExprVisitor {
    void* (*visitBinary)(Expr* expr, void* context);
    void* (*visitLogical)(Expr* expr, void* context);
    void* (*visitUnary)(Expr* expr, void* context);
    void* (*visitLiteral)(Expr* expr, void* context);
    void* (*visitGrouping)(Expr* expr, void* context);
    void* (*visitVariable)(Expr* expr, void* context);
    void* (*visitAssign)(Expr* expr, void* context);
};

/*
 * Centralized double-dispatch point
 */
void* exprAccept(Expr* expr, ExprVisitor* visitor, void* context);

/*
 * Factory functions (AST Node Constructors)
 */
Expr* newBinaryExpr(Expr* left, Token operator, Expr* right);
Expr* newLogicalExpr(Expr* left, Token operator, Expr* right);
Expr* newUnaryExpr(Token operator, Expr* right);
Expr* newLiteralExpr(Value value);
Expr* newGroupingExpr(Expr* expression);
Expr* newVariableExpr(Token name);
Expr* newAssignExpr(Token name, Expr* value);

/*
 * Destructor: Recursively frees an AST expression and all its child nodes
 */
void freeExpr(Expr* expr);

#endif /* TULX_EXPR_H */
