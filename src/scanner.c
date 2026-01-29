#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "scanner.h"
#include "error.h"
#include "keywords.h"

/* ---------- helpers ---------- */

static bool isAtEnd(Scanner* s) {
    return s->current >= s->length;
}

static char advance(Scanner* s) {
    return s->source[s->current++];
}

static char peek(Scanner* s) {
    return isAtEnd(s) ? '\0' : s->source[s->current];
}

static char peekNext(Scanner* s) {
    if (s->current + 1 >= s->length) return '\0';
    return s->source[s->current + 1];
}

static bool match(Scanner* s, char expected) {
    if (isAtEnd(s)) return false;
    if (s->source[s->current] != expected) return false;
    s->current++;
    return true;
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

static bool isAlphaNumeric(char c) {
    return isAlpha(c) || isDigit(c);
}

/* ---------- token storage ---------- */

static void addToken(
    Scanner* s,
    TokenType type,
    void* literal
) {
    if (s->count >= s->capacity) {
        s->capacity = s->capacity < 8 ? 8 : s->capacity * 2;
        s->tokens = realloc(s->tokens, sizeof(Token) * s->capacity);
    }

    s->tokens[s->count++] = makeToken(
        type,
        s->source + s->start,
        s->current - s->start,
        literal,
        s->line
    );
}

/* ---------- literals ---------- */

static void string(Scanner* s) {
    while (peek(s) != '"' && !isAtEnd(s)) {
        if (peek(s) == '\n') s->line++;
        advance(s);
    }

    if (isAtEnd(s)) {
        error(s->line, "Unterminated string.");
        return;
    }

    advance(s); // closing "

    int length = s->current - s->start - 2;
    char* value = malloc(length + 1);
    memcpy(value, s->source + s->start + 1, length);
    value[length] = '\0';

    addToken(s, TOKEN_STRING, value);
}

static void number(Scanner* s) {
    while (isDigit(peek(s))) advance(s);

    if (peek(s) == '.' && isDigit(peekNext(s))) {
        advance(s);
        while (isDigit(peek(s))) advance(s);
    }

    double* value = malloc(sizeof(double));
    *value = strtod(s->source + s->start, NULL);

    addToken(s, TOKEN_NUMBER, value);
}

static void identifier(Scanner* s) {
    while (isAlphaNumeric(peek(s))) advance(s);

    TokenType type = identifierType(
        s->source + s->start,
        s->current - s->start
    );

    addToken(s, type, NULL);
}

/* ---------- scan one token ---------- */

static void scanToken(Scanner* s) {
    char c = advance(s);

    switch (c) {
        case '(': addToken(s, TOKEN_LEFT_PAREN, NULL); break;
        case ')': addToken(s, TOKEN_RIGHT_PAREN, NULL); break;
        case '{': addToken(s, TOKEN_LEFT_BRACE, NULL); break;
        case '}': addToken(s, TOKEN_RIGHT_BRACE, NULL); break;
        case ',': addToken(s, TOKEN_COMMA, NULL); break;
        case '.': addToken(s, TOKEN_DOT, NULL); break;
        case '-': addToken(s, TOKEN_MINUS, NULL); break;
        case '+': addToken(s, TOKEN_PLUS, NULL); break;
        case ';': addToken(s, TOKEN_SEMICOLON, NULL); break;
        case '*': addToken(s, TOKEN_STAR, NULL); break;

        case '!': {
            bool eq = match(s, '=');
            addToken(s, eq ? TOKEN_BANG_EQUAL : TOKEN_BANG, NULL);
            break;
        }

        case '=': {
            bool eq = match(s, '=');
            addToken(s, eq ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL, NULL);
            break;
        }

        case '<': {
            bool eq = match(s, '=');
            addToken(s, eq ? TOKEN_LESS_EQUAL : TOKEN_LESS, NULL);
            break;
        }

        case '>': {
            bool eq = match(s, '=');
            addToken(s, eq ? TOKEN_GREATER_EQUAL : TOKEN_GREATER, NULL);
            break;
        }

        case '/':
            if (match(s, '/')) {
                while (peek(s) != '\n' && !isAtEnd(s)) advance(s);
            } else {
                addToken(s, TOKEN_SLASH, NULL);
            }
            break;

        case ' ':
        case '\r':
        case '\t':
            break;

        case '\n':
            s->line++;
            break;

        case '"':
            string(s);
            break;

        default:
            if (isDigit(c)) number(s);
            else if (isAlpha(c)) identifier(s);
            else error(s->line, "Unexpected character.");
            break;
    }
}

/* ---------- public API ---------- */

Scanner makeScanner(const char* source) {
    Scanner s;
    s.source = source;
    s.length = strlen(source);
    s.start = 0;
    s.current = 0;
    s.line = 1;
    s.tokens = NULL;
    s.count = 0;
    s.capacity = 0;
    return s;
}

void scanTokens(Scanner* s) {
    while (!isAtEnd(s)) {
        s->start = s->current;
        scanToken(s);
    }

    addToken(s, TOKEN_EOF, NULL);
}
