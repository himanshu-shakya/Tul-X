#ifndef TULX_STMT_H
#define TULX_STMT_H

#include "token.h"
#include "expr.h"

/* Forward declarations */
typedef struct Stmt Stmt;
typedef struct StmtVisitor StmtVisitor;

/*
 * Statement types supported in TUL-X
 */
typedef enum {
    STMT_EXPR,
    STMT_PRINT,
    STMT_VAR,
    STMT_BLOCK
} StmtType;

/*
 * Base Statement AST node (tagged union)
 */
struct Stmt {
    StmtType type;

    union {
        /* Expression statement: <expr>; */
        struct {
            Expr* expression;
        } expr;

        /* Print statement: print <expr>; */
        struct {
            Expr* expression;
        } print;

        /* Variable declaration: var <name> = <initializer>; */
        struct {
            Token name;
            Expr* initializer; /* can be NULL if uninitialized */
        } var;

        /* Block statement: { <stmt>* } */
        struct {
            Stmt** statements;
            int count;
        } block;
    } as;
};

/*
 * Statement Visitor interface
 */
struct StmtVisitor {
    void (*visitExpr)(Stmt* stmt, void* context);
    void (*visitPrint)(Stmt* stmt, void* context);
    void (*visitVar)(Stmt* stmt, void* context);
    void (*visitBlock)(Stmt* stmt, void* context);
};

/*
 * Centralized double-dispatch point for statements
 */
void stmtAccept(Stmt* stmt, StmtVisitor* visitor, void* context);

/*
 * Factory functions (AST Node Constructors)
 */
Stmt* newExprStmt(Expr* expression);
Stmt* newPrintStmt(Expr* expression);
Stmt* newVarStmt(Token name, Expr* initializer);
Stmt* newBlockStmt(Stmt** statements, int count);

/*
 * Destructors: Safely deallocate statement nodes
 */
void freeStmt(Stmt* stmt);
void freeStmtList(Stmt** statements, int count);

#endif /* TULX_STMT_H */
