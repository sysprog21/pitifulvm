#pragma once

#include "classfile.h"

typedef struct {
    variable_t *value;
    class_file_t *class;
    size_t fields_count;
} object_t;

typedef struct {
    u2 length;
    object_t **objects;
} object_heap_t;

void init_object_heap();
void free_object_heap();
object_t *create_object(class_file_t *clazz);