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
variable_t *find_field_addr(object_t *obj, char *name);