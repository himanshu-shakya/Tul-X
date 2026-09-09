#ifndef TULX_COMMON_H
#define TULX_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Exit codes based on sysexits.h standard:
 * 64 - Command line usage error
 * 65 - Data format / compile error (syntax, parsing)
 * 70 - Internal software error / runtime exception
 * 74 - Input/output error
 */
#define EX_USAGE   64
#define EX_DATAERR 65
#define EX_SOFTWARE 70
#define EX_IOERR   74

#endif /* TULX_COMMON_H */
