#include "common.h"
#include "error.h"
#include "scanner.h"
#include "token.h"
#include "parser.h"
#include "expr.h"

/* Forward declaration for ast printer */
char* printExpr(Expr* expr);

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

/* Scanner-only mode for token inspection */
static void runScannerOnly(const char* source) {
    Scanner scanner = makeScanner(source);
    scanTokens(&scanner);

    for (int i = 0; i < scanner.count; i++) {
        printToken(scanner.tokens[i]);
    }

    freeScanner(&scanner);
}

/* Full pipeline: Scanner -> Parser -> AST */
static void run(const char* source) {
    /* 1. Lexical Scanning */
    Scanner scanner = makeScanner(source);
    scanTokens(&scanner);

    if (hadError) {
        freeScanner(&scanner);
        return;
    }

    /* 2. Syntactic Parsing (Recursive Descent) */
    Parser parser = makeParser(scanner.tokens, scanner.count);
    Expr* expression = parse(&parser);

    /* Stop if there was a syntax/parse error */
    if (hadError || expression == NULL) {
        if (expression != NULL) freeExpr(expression);
        freeScanner(&scanner);
        return;
    }

    /* 3. AST Printing / Inspection */
    char* astRepresentation = printExpr(expression);
    if (astRepresentation != NULL) {
        printf("%s\n", astRepresentation);
        free(astRepresentation);
    }

    /* 4. Safe Resource Cleanup */
    freeExpr(expression);
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

static void runFile(const char* path, bool scanOnly) {
    const char* ext = strrchr(path, '.');

    if (!ext || strcmp(ext, ".tul") != 0) {
        fprintf(stderr, "Error: TUL-X source files must have the extension '.tul'. Provided: \"%s\"\n", path);
        exit(EX_USAGE);
    }

    char* source = readFile(path);
    if (scanOnly) {
        runScannerOnly(source);
    } else {
        run(source);
    }
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
            printf("Usage: tulx [options] [script.tul]\n");
            printf("Options:\n");
            printf("  --scan, -s     Run scanner only and print tokens\n");
            printf("  --help, -h     Show this help message\n");
            printf("Run without arguments to start the interactive REPL.\n");
            return 0;
        }
        runFile(argv[1], false);
    } else if (argc == 3 && (strcmp(argv[1], "--scan") == 0 || strcmp(argv[1], "-s") == 0)) {
        runFile(argv[2], true);
    } else {
        fprintf(stderr, "Usage: tulx [--scan] [script.tul]\n");
        exit(EX_USAGE);
    }

    return 0;
}
