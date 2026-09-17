#include "parser.h"
#include "error.h"
#include "common.h"

/* ============================================================================
 * Helper Prototypes
 * ============================================================================ */
static Stmt* declaration(Parser* p);
static Stmt* varDeclaration(Parser* p);
static Stmt* statement(Parser* p);
static Stmt* printStatement(Parser* p);
static Stmt* block(Parser* p);
static Stmt* exprStatement(Parser* p);

static Expr* expression(Parser* p);
static Expr* assignment(Parser* p);
static Expr* equality(Parser* p);
static Expr* comparison(Parser* p);
static Expr* term(Parser* p);
static Expr* factor(Parser* p);
static Expr* unary(Parser* p);
static Expr* primary(Parser* p);

/* ============================================================================
 * Parser Utility Functions
 * ============================================================================ */

static bool isAtEnd(Parser* p) {
    return p->current >= p->count || p->tokens[p->current].type == TOKEN_EOF;
}

static Token peek(Parser* p) {
    return p->tokens[p->current];
}

static Token previous(Parser* p) {
    return p->tokens[p->current - 1];
}

static Token advance(Parser* p) {
    if (!isAtEnd(p)) p->current++;
    return previous(p);
}

static bool check(Parser* p, TokenType type) {
    if (isAtEnd(p)) return false;
    return peek(p).type == type;
}

static bool match(Parser* p, TokenType type) {
    if (check(p, type)) {
        advance(p);
        return true;
    }
    return false;
}

static void errorAtToken(Token token, const char* message) {
    if (token.type == TOKEN_EOF) {
        report(token.line, " at end", message);
    } else {
        char where[256];
        snprintf(where, sizeof(where), " at '%.*s'", token.length, token.start);
        report(token.line, where, message);
    }
}

static Token consume(Parser* p, TokenType type, const char* message) {
    if (check(p, type)) return advance(p);

    errorAtToken(peek(p), message);
    return peek(p);
}

static void synchronize(Parser* p) {
    advance(p);

    while (!isAtEnd(p)) {
        if (previous(p).type == TOKEN_SEMICOLON) return;

        switch (peek(p).type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;
            default:
                break;
        }

        advance(p);
    }
}

static char* duplicateString(const char* src) {
    if (!src) return NULL;
    size_t len = strlen(src);
    char* copy = (char*)malloc(len + 1);
    if (!copy) return NULL;
    memcpy(copy, src, len + 1);
    return copy;
}

/* ============================================================================
 * Statement & Declaration Grammar
 * ============================================================================ */

static Stmt* declaration(Parser* p) {
    Stmt* stmt = NULL;
    if (match(p, TOKEN_VAR)) {
        stmt = varDeclaration(p);
    } else {
        stmt = statement(p);
    }

    if (hadError) {
        synchronize(p);
        return NULL;
    }

    return stmt;
}

static Stmt* varDeclaration(Parser* p) {
    Token name = consume(p, TOKEN_IDENTIFIER, "Expect variable name.");
    if (hadError) return NULL;

    Expr* initializer = NULL;
    if (match(p, TOKEN_EQUAL)) {
        initializer = expression(p);
    }

    consume(p, TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    if (hadError) {
        if (initializer) freeExpr(initializer);
        return NULL;
    }

    return newVarStmt(name, initializer);
}

static Stmt* statement(Parser* p) {
    if (match(p, TOKEN_PRINT)) {
        return printStatement(p);
    }
    if (match(p, TOKEN_LEFT_BRACE)) {
        return block(p);
    }
    return exprStatement(p);
}

static Stmt* printStatement(Parser* p) {
    Expr* value = expression(p);
    if (value == NULL) return NULL;

    consume(p, TOKEN_SEMICOLON, "Expect ';' after value.");
    if (hadError) {
        freeExpr(value);
        return NULL;
    }

    return newPrintStmt(value);
}

static Stmt* block(Parser* p) {
    int capacity = 8;
    int count = 0;
    Stmt** statements = (Stmt**)malloc(capacity * sizeof(Stmt*));
    if (!statements) {
        fprintf(stderr, "Out of memory in block parser.\n");
        exit(EX_SOFTWARE);
    }

    while (!check(p, TOKEN_RIGHT_BRACE) && !isAtEnd(p)) {
        Stmt* stmt = declaration(p);
        if (stmt != NULL) {
            if (count >= capacity) {
                capacity *= 2;
                Stmt** resized = (Stmt**)realloc(statements, capacity * sizeof(Stmt*));
                if (!resized) {
                    fprintf(stderr, "Out of memory resizing block statements.\n");
                    exit(EX_SOFTWARE);
                }
                statements = resized;
            }
            statements[count++] = stmt;
        }
    }

    consume(p, TOKEN_RIGHT_BRACE, "Expect '}' after block.");
    if (hadError) {
        freeStmtList(statements, count);
        return NULL;
    }

    return newBlockStmt(statements, count);
}

static Stmt* exprStatement(Parser* p) {
    Expr* expr = expression(p);
    if (expr == NULL) return NULL;

    if (match(p, TOKEN_SEMICOLON) || isAtEnd(p)) {
        return newExprStmt(expr);
    }

    consume(p, TOKEN_SEMICOLON, "Expect ';' after expression.");
    if (hadError) {
        freeExpr(expr);
        return NULL;
    }

    return newExprStmt(expr);
}

/* ============================================================================
 * Expression Grammar Rules (Recursive Descent)
 *
 * Precedence hierarchy (lowest to highest):
 *   expression -> assignment
 *   assignment -> IDENTIFIER "=" assignment | equality
 *   equality   -> comparison ( ("!=" | "==") comparison )*
 *   comparison -> term ( (">" | ">=" | "<" | "<=") term )*
 *   term       -> factor ( ("-" | "+") factor )*
 *   factor     -> unary ( ("/" | "*") unary )*
 *   unary      -> ("!" | "-") unary | primary
 *   primary    -> NUMBER | STRING | "true" | "false" | "null" | IDENTIFIER | "(" expression ")"
 * ============================================================================ */

static Expr* expression(Parser* p) {
    return assignment(p);
}

static Expr* assignment(Parser* p) {
    Expr* expr = equality(p);

    if (match(p, TOKEN_EQUAL)) {
        Token equals = previous(p);
        Expr* value = assignment(p);

        if (expr != NULL && expr->type == EXPR_VARIABLE) {
            Token name = expr->as.variable.name;
            free(expr); /* Free the Expr wrapper shell, Token points into source buffer */
            return newAssignExpr(name, value);
        }

        errorAtToken(equals, "Invalid assignment target.");
        if (value) freeExpr(value);
        if (expr) freeExpr(expr);
        return NULL;
    }

    return expr;
}

static Expr* equality(Parser* p) {
    Expr* expr = comparison(p);
    if (expr == NULL) return NULL;

    while (match(p, TOKEN_BANG_EQUAL) || match(p, TOKEN_EQUAL_EQUAL)) {
        Token operator = previous(p);
        Expr* right = comparison(p);
        if (right == NULL) {
            freeExpr(expr);
            return NULL;
        }
        expr = newBinaryExpr(expr, operator, right);
    }

    return expr;
}

static Expr* comparison(Parser* p) {
    Expr* expr = term(p);
    if (expr == NULL) return NULL;

    while (match(p, TOKEN_GREATER) || match(p, TOKEN_GREATER_EQUAL) ||
           match(p, TOKEN_LESS) || match(p, TOKEN_LESS_EQUAL)) {
        Token operator = previous(p);
        Expr* right = term(p);
        if (right == NULL) {
            freeExpr(expr);
            return NULL;
        }
        expr = newBinaryExpr(expr, operator, right);
    }

    return expr;
}

static Expr* term(Parser* p) {
    Expr* expr = factor(p);
    if (expr == NULL) return NULL;

    while (match(p, TOKEN_MINUS) || match(p, TOKEN_PLUS)) {
        Token operator = previous(p);
        Expr* right = factor(p);
        if (right == NULL) {
            freeExpr(expr);
            return NULL;
        }
        expr = newBinaryExpr(expr, operator, right);
    }

    return expr;
}

static Expr* factor(Parser* p) {
    Expr* expr = unary(p);
    if (expr == NULL) return NULL;

    while (match(p, TOKEN_SLASH) || match(p, TOKEN_STAR)) {
        Token operator = previous(p);
        Expr* right = unary(p);
        if (right == NULL) {
            freeExpr(expr);
            return NULL;
        }
        expr = newBinaryExpr(expr, operator, right);
    }

    return expr;
}

static Expr* unary(Parser* p) {
    if (match(p, TOKEN_BANG) || match(p, TOKEN_MINUS)) {
        Token operator = previous(p);
        Expr* right = unary(p);
        if (right == NULL) return NULL;
        return newUnaryExpr(operator, right);
    }

    return primary(p);
}

static Expr* primary(Parser* p) {
    if (match(p, TOKEN_FALSE)) return newLiteralExpr(BOOL_VAL(false));
    if (match(p, TOKEN_TRUE))  return newLiteralExpr(BOOL_VAL(true));
    if (match(p, TOKEN_NULL))  return newLiteralExpr(NULL_VAL());

    if (match(p, TOKEN_NUMBER)) {
        double val = *(double*)previous(p).literal;
        return newLiteralExpr(NUMBER_VAL(val));
    }

    if (match(p, TOKEN_STRING)) {
        char* strCopy = duplicateString((const char*)previous(p).literal);
        return newLiteralExpr(STRING_VAL(strCopy));
    }

    if (match(p, TOKEN_IDENTIFIER)) {
        return newVariableExpr(previous(p));
    }

    if (match(p, TOKEN_LEFT_PAREN)) {
        Expr* expr = expression(p);
        if (expr == NULL) return NULL;

        consume(p, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
        if (hadError) {
            freeExpr(expr);
            return NULL;
        }
        return newGroupingExpr(expr);
    }

    errorAtToken(peek(p), "Expect expression.");
    return NULL;
}

/* ============================================================================
 * Public Parser API
 * ============================================================================ */

Parser makeParser(Token* tokens, int count) {
    Parser p;
    p.tokens = tokens;
    p.count = count;
    p.current = 0;
    return p;
}

Stmt** parse(Parser* p, int* outCount) {
    if (p->count == 0 || isAtEnd(p)) {
        if (outCount) *outCount = 0;
        return NULL;
    }

    int capacity = 8;
    int count = 0;
    Stmt** statements = (Stmt**)malloc(capacity * sizeof(Stmt*));
    if (!statements) {
        fprintf(stderr, "Out of memory in parser.\n");
        exit(EX_SOFTWARE);
    }

    while (!isAtEnd(p)) {
        Stmt* stmt = declaration(p);
        if (stmt != NULL) {
            if (count >= capacity) {
                capacity *= 2;
                Stmt** resized = (Stmt**)realloc(statements, capacity * sizeof(Stmt*));
                if (!resized) {
                    fprintf(stderr, "Out of memory resizing statements.\n");
                    exit(EX_SOFTWARE);
                }
                statements = resized;
            }
            statements[count++] = stmt;
        }
    }

    if (hadError) {
        freeStmtList(statements, count);
        if (outCount) *outCount = 0;
        return NULL;
    }

    if (outCount) *outCount = count;
    return statements;
}

Expr* parseExpressionOnly(Parser* p) {
    if (p->count == 0 || isAtEnd(p)) return NULL;

    Expr* expr = expression(p);
    if (hadError) {
        if (expr != NULL) freeExpr(expr);
        return NULL;
    }
    return expr;
}
