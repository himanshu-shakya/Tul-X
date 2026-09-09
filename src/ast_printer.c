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
    if (expr->as.literal.value == NULL) {
        return format("nil");
    }
    double* value = (double*)expr->as.literal.value;
    return format("%g", *value);
}

static void* printGrouping(Expr* expr) {
    char* inner = (char*)exprAccept(expr->as.grouping.expression, &printer);
    char* result = format("(group %s)", inner);
    free(inner);
    return result;
}

static void* printUnary(Expr* expr) {
    char* right = (char*)exprAccept(expr->as.unary.right, &printer);
    char* result = format("(%.*s %s)",
                          expr->as.unary.operator.length,
                          expr->as.unary.operator.start,
                          right);
    free(right);
    return result;
}

static void* printBinary(Expr* expr) {
    char* left = (char*)exprAccept(expr->as.binary.left, &printer);
    char* right = (char*)exprAccept(expr->as.binary.right, &printer);
    char* result = format("(%.*s %s %s)",
                          expr->as.binary.operator.length,
                          expr->as.binary.operator.start,
                          left,
                          right);
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
    return (char*)exprAccept(expr, &printer);
}