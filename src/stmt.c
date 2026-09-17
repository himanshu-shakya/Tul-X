#include "stmt.h"
#include "common.h"

/*
 * Centralized double-dispatch for statements
 */
void stmtAccept(Stmt* stmt, StmtVisitor* visitor, void* context) {
    if (stmt == NULL) return;

    switch (stmt->type) {
        case STMT_EXPR:
            visitor->visitExpr(stmt, context);
            break;
        case STMT_PRINT:
            visitor->visitPrint(stmt, context);
            break;
        case STMT_VAR:
            visitor->visitVar(stmt, context);
            break;
        case STMT_BLOCK:
            visitor->visitBlock(stmt, context);
            break;
        case STMT_IF:
            visitor->visitIf(stmt, context);
            break;
        case STMT_WHILE:
            visitor->visitWhile(stmt, context);
            break;
    }
}

/*
 * Factory functions (AST Node Constructors)
 */

Stmt* newExprStmt(Expr* expression) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    if (!stmt) {
        fprintf(stderr, "Out of memory in newExprStmt.\n");
        exit(EX_SOFTWARE);
    }
    stmt->type = STMT_EXPR;
    stmt->as.expr.expression = expression;
    return stmt;
}

Stmt* newPrintStmt(Expr* expression) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    if (!stmt) {
        fprintf(stderr, "Out of memory in newPrintStmt.\n");
        exit(EX_SOFTWARE);
    }
    stmt->type = STMT_PRINT;
    stmt->as.print.expression = expression;
    return stmt;
}

Stmt* newVarStmt(Token name, Expr* initializer) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    if (!stmt) {
        fprintf(stderr, "Out of memory in newVarStmt.\n");
        exit(EX_SOFTWARE);
    }
    stmt->type = STMT_VAR;
    stmt->as.var.name = name;
    stmt->as.var.initializer = initializer;
    return stmt;
}

Stmt* newBlockStmt(Stmt** statements, int count) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    if (!stmt) {
        fprintf(stderr, "Out of memory in newBlockStmt.\n");
        exit(EX_SOFTWARE);
    }
    stmt->type = STMT_BLOCK;
    stmt->as.block.statements = statements;
    stmt->as.block.count = count;
    return stmt;
}

Stmt* newIfStmt(Expr* condition, Stmt* thenBranch, Stmt* elseBranch) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    if (!stmt) {
        fprintf(stderr, "Out of memory in newIfStmt.\n");
        exit(EX_SOFTWARE);
    }
    stmt->type = STMT_IF;
    stmt->as.ifStmt.condition = condition;
    stmt->as.ifStmt.thenBranch = thenBranch;
    stmt->as.ifStmt.elseBranch = elseBranch;
    return stmt;
}

Stmt* newWhileStmt(Expr* condition, Stmt* body) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    if (!stmt) {
        fprintf(stderr, "Out of memory in newWhileStmt.\n");
        exit(EX_SOFTWARE);
    }
    stmt->type = STMT_WHILE;
    stmt->as.whileStmt.condition = condition;
    stmt->as.whileStmt.body = body;
    return stmt;
}

/*
 * Destructors: Safely and recursively free statement nodes
 */
void freeStmt(Stmt* stmt) {
    if (stmt == NULL) return;

    switch (stmt->type) {
        case STMT_EXPR:
            freeExpr(stmt->as.expr.expression);
            break;
        case STMT_PRINT:
            freeExpr(stmt->as.print.expression);
            break;
        case STMT_VAR:
            if (stmt->as.var.initializer != NULL) {
                freeExpr(stmt->as.var.initializer);
            }
            break;
        case STMT_BLOCK:
            freeStmtList(stmt->as.block.statements, stmt->as.block.count);
            break;
        case STMT_IF:
            freeExpr(stmt->as.ifStmt.condition);
            if (stmt->as.ifStmt.thenBranch != NULL) {
                freeStmt(stmt->as.ifStmt.thenBranch);
            }
            if (stmt->as.ifStmt.elseBranch != NULL) {
                freeStmt(stmt->as.ifStmt.elseBranch);
            }
            break;
        case STMT_WHILE:
            freeExpr(stmt->as.whileStmt.condition);
            if (stmt->as.whileStmt.body != NULL) {
                freeStmt(stmt->as.whileStmt.body);
            }
            break;
    }

    free(stmt);
}

void freeStmtList(Stmt** statements, int count) {
    if (statements == NULL) return;
    for (int i = 0; i < count; i++) {
        if (statements[i] != NULL) {
            freeStmt(statements[i]);
        }
    }
    free(statements);
}
