#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include "boolean.h"

extern BOOLEAN debug_enabled;

#define DEBUG_PRINT(fmt, ...) \
    do { \
        if (debug_enabled) { \
            fprintf(stderr, "[DEBUG] " fmt "\n", ##__VA_ARGS__); \
        } \
    } while(0)

#define DEBUG_INIT(enable) \
    do { \
        debug_enabled = enable; \
        if (debug_enabled) { \
            DEBUG_PRINT("Debug mode enabled"); \
        } \
    } while(0)

#endif /* DEBUG_H */ 