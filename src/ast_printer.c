#include "common.h"
#include "expr.h"
#include <stdarg.h>

static ExprVisitor printer;

static char* format(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    va_end(args);

    size_t len = strlen(buffer);
    char* copy = (char*)malloc(len + 1);
    if (!copy) return NULL;
    memcpy(copy, buffer, len + 1);
    return copy;
}

static void* printLiteral(Expr* expr) {
    Value value = expr->as.literal.value;
    switch (value.type) {
        case VAL_NULL:
            return format("null");
        case VAL_BOOL:
            return format(AS_BOOL(value) ? "true" : "false");
        case VAL_NUMBER:
            return format("%g", AS_NUMBER(value));
        case VAL_STRING:
            return format("\"%s\"", AS_STRING(value));
    }
    return format("<?>");
}

static void* printGrouping(Expr* expr) {
    char* inner = (char*)exprAccept(expr->as.grouping.expression, &printer);
    char* result = format("(group %s)", inner ? inner : "nil");
    free(inner);
    return result;
}

static void* printUnary(Expr* expr) {
    char* right = (char*)exprAccept(expr->as.unary.right, &printer);
    char* result = format("(%.*s %s)",
                          expr->as.unary.operator.length,
                          expr->as.unary.operator.start,
                          right ? right : "nil");
    free(right);
    return result;
}

static void* printBinary(Expr* expr) {
    char* left = (char*)exprAccept(expr->as.binary.left, &printer);
    char* right = (char*)exprAccept(expr->as.binary.right, &printer);
    char* result = format("(%.*s %s %s)",
                          expr->as.binary.operator.length,
                          expr->as.binary.operator.start,
                          left ? left : "nil",
                          right ? right : "nil");
    free(left);
    free(right);
    return result;
}

static ExprVisitor printer = {
    .visitBinary   = printBinary,
    .visitUnary    = printUnary,
    .visitLiteral  = printLiteral,
    .visitGrouping = printGrouping
};

char* printExpr(Expr* expr) {
    if (expr == NULL) return NULL;
    return (char*)exprAccept(expr, &printer);
}