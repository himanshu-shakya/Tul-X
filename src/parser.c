#include "parser.h"
#include "error.h"
#include "common.h"

/* ============================================================================
 * Helper Prototypes
 * ============================================================================ */
static Expr* expression(Parser* p);
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
 * Grammar Rules (Recursive Descent)
 *
 * Precedence hierarchy (lowest to highest):
 *   expression -> equality
 *   equality   -> comparison ( ("!=" | "==") comparison )*
 *   comparison -> term ( (">" | ">=" | "<" | "<=") term )*
 *   term       -> factor ( ("-" | "+") factor )*
 *   factor     -> unary ( ("/" | "*") unary )*
 *   unary      -> ("!" | "-") unary | primary
 *   primary    -> NUMBER | STRING | "true" | "false" | "null" | "(" expression ")"
 * ============================================================================ */

static Expr* expression(Parser* p) {
    return equality(p);
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

Expr* parse(Parser* p) {
    if (p->count == 0 || isAtEnd(p)) return NULL;

    Expr* expr = expression(p);

    if (hadError) {
        synchronize(p);
        if (expr != NULL) {
            freeExpr(expr);
            expr = NULL;
        }
    }

    return expr;
}
