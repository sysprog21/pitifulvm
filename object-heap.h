#pragma once

#include <string.h>

#include "classfile.h"
#include "list.h"

typedef struct object {
    variable_t *value;
    class_file_t *class;
    size_t fields_count;
    struct object *parent;
} object_t;

typedef struct {
    u2 length;
    object_t **objects;
} object_heap_t;

void init_object_heap();
void free_object_heap();
object_t *create_object(class_file_t *clazz);
char *create_string(class_file_t *clazz, char *src);
void **build_array(uint8_t depth,
                   int dimension,
                   int *n_elements,
                   size_t type_size);
void free_array(object_t *obj, uint8_t depth, int dimension, void **arr);
void *create_array(class_file_t *clazz,
                   uint8_t dimension,
                   int *dimensions,
                   size_t type_size);
variable_t *find_field_addr(object_t *obj, char *name);