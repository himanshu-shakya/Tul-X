#include "scanner.h"
#include "common.h"
#include "error.h"
#include "keywords.h"

/* ============================================================================
 * Internal Scanner Helper Functions
 * ============================================================================ */

static bool isAtEnd(Scanner* s) {
    return s->current >= s->length;
}

static char advance(Scanner* s) {
    return s->source[s->current++];
}

static char peek(Scanner* s) {
    if (isAtEnd(s)) return '\0';
    return s->source[s->current];
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

/* ============================================================================
 * Token Storage (Dynamic Array)
 * ============================================================================ */

static void addToken(Scanner* s, TokenType type, void* literal) {
    if (s->count >= s->capacity) {
        s->capacity = s->capacity < 8 ? 8 : s->capacity * 2;
        Token* newTokens = (Token*)realloc(s->tokens, sizeof(Token) * s->capacity);
        if (!newTokens) {
            fprintf(stderr, "Out of memory in scanner token buffer.\n");
            exit(EX_SOFTWARE);
        }
        s->tokens = newTokens;
    }

    s->tokens[s->count++] = makeToken(
        type,
        s->source + s->start,
        s->current - s->start,
        literal,
        s->line
    );
}

/* ============================================================================
 * Literal Scanning
 * ============================================================================ */

static void string(Scanner* s) {
    while (peek(s) != '"' && !isAtEnd(s)) {
        if (peek(s) == '\n') {
            s->line++;
        }
        advance(s);
    }

    if (isAtEnd(s)) {
        error(s->line, "Unterminated string.");
        return;
    }

    /* Consume the closing quote */
    advance(s);

    /* Extract string contents excluding surrounding quotes */
    int length = s->current - s->start - 2;
    char* value = (char*)malloc(length + 1);
    if (!value) {
        fprintf(stderr, "Out of memory while allocating string literal.\n");
        exit(EX_SOFTWARE);
    }

    memcpy(value, s->source + s->start + 1, length);
    value[length] = '\0';

    addToken(s, TOKEN_STRING, value);
}

static void number(Scanner* s) {
    while (isDigit(peek(s))) {
        advance(s);
    }

    /* Look for fractional part (decimal point followed by at least one digit) */
    if (peek(s) == '.' && isDigit(peekNext(s))) {
        /* Consume the '.' */
        advance(s);

        while (isDigit(peek(s))) {
            advance(s);
        }
    }

    double* value = (double*)malloc(sizeof(double));
    if (!value) {
        fprintf(stderr, "Out of memory while allocating number literal.\n");
        exit(EX_SOFTWARE);
    }

    *value = strtod(s->source + s->start, NULL);
    addToken(s, TOKEN_NUMBER, value);
}

static void identifier(Scanner* s) {
    while (isAlphaNumeric(peek(s))) {
        advance(s);
    }

    TokenType type = identifierType(
        s->source + s->start,
        s->current - s->start
    );

    addToken(s, type, NULL);
}

/* ============================================================================
 * Scan Single Token
 * ============================================================================ */

static void scanToken(Scanner* s) {
    char c = advance(s);

    switch (c) {
        /* Single-character tokens */
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

        /* One or two character tokens */
        case '!':
            addToken(s, match(s, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG, NULL);
            break;

        case '=':
            addToken(s, match(s, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL, NULL);
            break;

        case '<':
            addToken(s, match(s, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS, NULL);
            break;

        case '>':
            addToken(s, match(s, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER, NULL);
            break;

        /* Slashes and Comments */
        case '/':
            if (match(s, '/')) {
                /* A single-line comment goes until the end of the line */
                while (peek(s) != '\n' && !isAtEnd(s)) {
                    advance(s);
                }
            } else {
                addToken(s, TOKEN_SLASH, NULL);
            }
            break;

        /* Whitespace characters */
        case ' ':
        case '\r':
        case '\t':
            /* Ignore whitespace */
            break;

        case '\n':
            s->line++;
            break;

        /* String Literals */
        case '"':
            string(s);
            break;

        default:
            if (isDigit(c)) {
                number(s);
            } else if (isAlpha(c)) {
                identifier(s);
            } else {
                error(s->line, "Unexpected character.");
            }
            break;
    }
}

/* ============================================================================
 * Public Scanner API
 * ============================================================================ */

Scanner makeScanner(const char* source) {
    Scanner s;
    s.source = source;
    s.length = (int)strlen(source);
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

    /* Append sentinel TOKEN_EOF */
    s->start = s->current;
    addToken(s, TOKEN_EOF, NULL);
}

void freeScanner(Scanner* s) {
    if (s == NULL || s->tokens == NULL) return;

    for (int i = 0; i < s->count; i++) {
        if (s->tokens[i].literal != NULL) {
            free(s->tokens[i].literal);
            s->tokens[i].literal = NULL;
        }
    }

    free(s->tokens);
    s->tokens = NULL;
    s->count = 0;
    s->capacity = 0;
}
