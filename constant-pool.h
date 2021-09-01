#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"

typedef enum {
    CONSTANT_Utf8 = 1,
    CONSTANT_Integer = 3,
    CONSTANT_Long = 5,
    CONSTANT_Class = 7,
    CONSTANT_String = 8,
    CONSTANT_FieldRef = 9,
    CONSTANT_MethodRef = 10,
    CONSTANT_NameAndType = 12,
    CONSTANT_MethodHandle = 15,
    CONSTANT_InvokeDynamic = 18,
} const_pool_tag_t;

typedef struct {
    u2 string_index;
} CONSTANT_Class_info;

typedef struct {
    u2 class_index;
    u2 name_and_type_index;
} CONSTANT_FieldOrMethodRef_info;

typedef struct {
    int32_t bytes;
} CONSTANT_Integer_info;

typedef struct {
    u4 high_bytes;
    u4 low_bytes;
} CONSTANT_LongOrDouble_info;

typedef struct {
    u2 name_index;
    u2 descriptor_index;
} CONSTANT_NameAndType_info;

typedef struct {
    u2 string_index;
} CONSTANT_String_info;

typedef struct {
    u2 bootstrap_method_attr_index;
    u2 name_and_type_index;
} CONSTANT_InvokeDynamic_info;

typedef struct {
    u1 reference_kind;
    u2 reference_index;
} CONSTANT_MethodHandle_info;

typedef struct {
    const_pool_tag_t tag;
    u1 *info;
} const_pool_info;

typedef struct {
    u2 count;
    const_pool_info *constant_pool;
} constant_pool_t;

u1 read_u1(FILE *class_file);
u2 read_u2(FILE *class_file);
u4 read_u4(FILE *class_file);
const_pool_info *get_constant(constant_pool_t *constant_pool, u2 index);
constant_pool_t get_constant_pool(FILE *class_file);
CONSTANT_FieldOrMethodRef_info *get_methodref(constant_pool_t *cp, u2 idx);
CONSTANT_Class_info *get_class_name(constant_pool_t *cp, u2 idx);
CONSTANT_MethodHandle_info *get_method_handle(constant_pool_t *cp, u2 idx);
char *get_string_utf(constant_pool_t *cp, u2 idx);
