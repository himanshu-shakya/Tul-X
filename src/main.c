#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scanner.h"
#include "token.h"
#include "tokentype.h"
#include "error.h"

/* =============================
   Error state
   ============================= */
static int hadError = 0;

/* =============================
   Error reporting
   ============================= */
static void report(int line, const char* where, const char* message) {
    fprintf(stderr, "[line %d] Error%s: %s\n", line, where, message);
    hadError = 1;
}

void error(int line, const char* message) {
    report(line, "", message);
}

/* =============================
   Run source through scanner
   ============================= */
static void run(const char* source) {
    Scanner scanner = makeScanner(source);
    scanTokens(&scanner);

    /* For now: just print tokens */
    for (int i = 0; i < scanner.count; i++) {
        printToken(scanner.tokens[i]);
    }

    /* TODO later: free tokens & lexemes */
}

/* =============================
   REPL
   ============================= */
static void repl() {
    char line[1024];

    for (;;) {
        printf("tul-x > ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        run(line);
        hadError = 0;
    }
}

/* =============================
   Run file
   ============================= */
static void runFile(const char* path) {
    const char* ext = strrchr(path, '.');

    if (!ext || strcmp(ext, ".tlx") != 0) {
        fprintf(stderr, "Can only run .tlx files.\n");
        exit(64);
    }

    FILE* file = fopen(path, "rb");
    if (!file) {
        perror("Could not open file");
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char* source = malloc(size + 1);
    if (!source) {
        fprintf(stderr, "Out of memory.\n");
        exit(74);
    }

    fread(source, 1, size, file);
    source[size] = '\0';
    fclose(file);

    run(source);
    free(source);

    if (hadError) exit(65);
}

/* =============================
   Main
   ============================= */
int main(int argc, const char* argv[]) {
    if (argc > 2) {
        fprintf(stderr, "Usage: tulx [file.tlx]\n");
        exit(64);
    }

    if (argc == 2) {
        runFile(argv[1]);
    } else {
        repl();
    }

    return 0;
}
