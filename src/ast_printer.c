#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "expr.h"

/*
 * Forward declaration of the visitor.
 * Required because visitor methods call exprAccept() recursively.
 */
static ExprVisitor printer;

/*
 * Utility function
 *
 * Creates a heap-allocated formatted string.
 * Similar to Java's String.format().
 */
static char* format(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    va_end(args);
    return strdup(buffer);
}

/*
 * visitLiteral
 *
 * Java equivalent:
 *   String visitLiteralExpr(Literal expr)
 */
static void* printLiteral(Expr* expr) {
    /* For now assume numeric literals */
    double* value = (double*)expr->as.literal.value;
    return format("%g", *value);
}

/*
 * visitGrouping
 *
 * Java equivalent:
 *   String visitGroupingExpr(Grouping expr)
 */
static void* printGrouping(Expr* expr) {
    char* inner =
        (char*)exprAccept(expr->as.grouping.expression, &printer);

    char* result = format("(group %s)", inner);
    free(inner);
    return result;
}

/*
 * visitUnary
 *
 * Java equivalent:
 *   String visitUnaryExpr(Unary expr)
 */
static void* printUnary(Expr* expr) {
    char* right =
        (char*)exprAccept(expr->as.unary.right, &printer);

    char* result =
        format("(%s %s)",
               expr->as.unary.operator.lexeme,
               right);

    free(right);
    return result;
}

/*
 * visitBinary
 *
 * Java equivalent:
 *   String visitBinaryExpr(Binary expr)
 */
static void* printBinary(Expr* expr) {
    char* left =
        (char*)exprAccept(expr->as.binary.left, &printer);

    char* right =
        (char*)exprAccept(expr->as.binary.right, &printer);

    char* result =
        format("(%s %s %s)",
               expr->as.binary.operator.lexeme,
               left,
               right);

    free(left);
    free(right);
    return result;
}

/*
 * The AstPrinter visitor instance
 *
 * Java equivalent:
 *   class AstPrinter implements Expr.Visitor<String>
 */
static ExprVisitor printer = {
    .visitBinary   = printBinary,
    .visitUnary    = printUnary,
    .visitLiteral  = printLiteral,
    .visitGrouping = printGrouping
};

/*
 * Public entry point
 *
 * Java equivalent:
 *   printer.print(expr)
 */
char* printExpr(Expr* expr) {
    return (char*)exprAccept(expr, &printer);
}