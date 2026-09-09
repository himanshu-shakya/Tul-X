#include "common.h"
#include "error.h"
#include "scanner.h"
#include "token.h"

/* ============================================================================
 * Global Error State
 * ============================================================================ */
bool hadError = false;
bool hadRuntimeError = false;

/* ============================================================================
 * Error Reporting
 * ============================================================================ */
void report(int line, const char* where, const char* message) {
    fprintf(stderr, "[line %d] Error%s: %s\n", line, where, message);
    hadError = true;
}

void error(int line, const char* message) {
    report(line, "", message);
}

/* ============================================================================
 * Pipeline Execution
 * ============================================================================ */
static void run(const char* source) {
    Scanner scanner = makeScanner(source);
    scanTokens(&scanner);

    /* Phase 0/1: display tokens for verification */
    for (int i = 0; i < scanner.count; i++) {
        printToken(scanner.tokens[i]);
    }

    freeScanner(&scanner);
}

/* ============================================================================
 * Interactive REPL (Read-Eval-Print Loop)
 * ============================================================================ */
static void repl(void) {
    char line[1024];

    printf("==========================================\n");
    printf("  TUL-X Interpreter (Crafting Interpreters)\n");
    printf("  Type 'exit' or press Ctrl+D to quit.\n");
    printf("==========================================\n");

    for (;;) {
        printf("tul-x > ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        /* Check for explicit exit command */
        if (strncmp(line, "exit", 4) == 0 && (line[4] == '\n' || line[4] == '\0' || line[4] == '\r')) {
            break;
        }

        run(line);
        hadError = false;
    }
}

/* ============================================================================
 * Run Source File
 * ============================================================================ */
static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(EX_IOERR);
    }

    fseek(file, 0L, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    if (!buffer) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        fclose(file);
        exit(EX_IOERR);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < (size_t)fileSize && ferror(file)) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        free(buffer);
        fclose(file);
        exit(EX_IOERR);
    }

    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}

static void runFile(const char* path) {
    const char* ext = strrchr(path, '.');

    if (!ext || strcmp(ext, ".tul") != 0) {
        fprintf(stderr, "Error: TUL-X source files must have the extension '.tul'. Provided: \"%s\"\n", path);
        exit(EX_USAGE);
    }

    char* source = readFile(path);
    run(source);
    free(source);

    if (hadError) exit(EX_DATAERR);
    if (hadRuntimeError) exit(EX_SOFTWARE);
}

/* ============================================================================
 * Main Entry Point
 * ============================================================================ */
int main(int argc, const char* argv[]) {
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("Usage: tulx [script.tul]\n");
            printf("Run without arguments to start the interactive REPL.\n");
            return 0;
        }
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: tulx [script.tul]\n");
        exit(EX_USAGE);
    }

    return 0;
}
