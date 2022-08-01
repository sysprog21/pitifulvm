#pragma once

#include "classfile.h"

typedef struct {
    u2 length;
    meta_class_t **class_info;
} class_heap_t;

void init_class_heap();
void free_class_heap();
void add_class(class_file_t *clazz, char *name);
class_file_t *find_class_from_heap(char *value);
bool find_or_add_class_to_heap(char *class_name,
                               char *prefix,
                               class_file_t **target_class);
char *find_method_info_from_index(uint16_t idx,
                                  class_file_t *clazz,
                                  char **name_info,
                                  char **descriptor_info);