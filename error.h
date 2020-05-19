#pragma once
#if !defined(ERROR_H)
#define ERROR_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static void error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "Error:\n");
    vfprintf(stderr, fmt, args);
    va_end(args);

    exit(-1);
}

#endif