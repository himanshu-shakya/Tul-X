#include <string.h>
#include "keywords.h"

typedef struct {
    const char* keyword;
    TokenType type;
} Keyword;

static const Keyword keywords[] = {
    {"and", TOKEN_AND},
    {"class", TOKEN_CLASS},
    {"else", TOKEN_ELSE},
    {"false", TOKEN_FALSE},
    {"for", TOKEN_FOR},
    {"fun", TOKEN_FUN},
    {"if", TOKEN_IF},
    {"null", TOKEN_NULL},
    {"or", TOKEN_OR},
    {"print", TOKEN_PRINT},
    {"return", TOKEN_RETURN},
    {"super", TOKEN_SUPER},
    {"this", TOKEN_THIS},
    {"true", TOKEN_TRUE},
    {"var", TOKEN_VAR},
    {"while", TOKEN_WHILE},
};

static int count = sizeof(keywords) / sizeof(keywords[0]);

TokenType identifierType(const char* start, int length) {
    for (int i = 0; i < count; i++) {
        if ((int)strlen(keywords[i].keyword) == length &&
            memcmp(start, keywords[i].keyword, length) == 0) {
            return keywords[i].type;
        }
    }
    return TOKEN_IDENTIFIER;
}
