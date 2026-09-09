#include "value.h"

void printValue(Value value) {
    switch (value.type) {
        case VAL_NULL:
            printf("null");
            break;
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case VAL_NUMBER:
            printf("%g", AS_NUMBER(value));
            break;
        case VAL_STRING:
            printf("%s", AS_STRING(value));
            break;
    }
}

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) return false;

    switch (a.type) {
        case VAL_NULL:   return true;
        case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_STRING: return strcmp(AS_STRING(a), AS_STRING(b)) == 0;
        default:         return false;
    }
}

void freeValue(Value value) {
    if (IS_STRING(value) && AS_STRING(value) != NULL) {
        free(AS_STRING(value));
    }
}
