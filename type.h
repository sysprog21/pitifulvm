#pragma once

#include <stdint.h>

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

typedef enum {
    VAR_NONE = 0,
    VAR_BYTE = 1,
    VAR_SHORT = 2,
    VAR_INT = 3,
    VAR_LONG = 4,
    VAR_PTR = 5,       /* reference */
    VAR_STR_PTR = 6,   /* string reference */
    VAR_ARRAY_PTR = 7, /* array reference */
} variable_type_t;

typedef union {
    u1 char_value;
    u2 short_value;
    u4 int_value;
    u8 long_value;
    void *ptr_value;
} value_t;

typedef struct {
    value_t value;
    variable_type_t type;
} variable_t;

typedef enum {
    T_BOOLEN = 4,
    T_CHAR = 5,
    T_FLOAT = 6,
    T_DOUBLE = 7,
    T_BYTE = 8,
    T_SHORT = 9,
    T_INT = 10,
    T_LONG = 11
} array_type_t;