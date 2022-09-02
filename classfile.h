#pragma once

#include "constant-pool.h"
#include "type.h"

typedef struct {
    u4 magic;
    u2 minor_version;
    u2 major_version;
} class_header_t;

typedef struct {
    u2 access_flags;
    u2 this_class;
    u2 super_class;
} class_info_t;

typedef struct {
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    u2 attributes_count;
} method_info;

typedef struct {
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    u2 attributes_count;
} field_info;

typedef struct {
    u2 attribute_name_index;
    u4 attribute_length;
} attribute_info;

typedef struct {
    u2 max_stack;
    u2 max_locals;
    u4 code_length;
    u1 *code;
} code_t;

typedef struct {
    char *name;
    char *descriptor;
    code_t code;
} method_t;

typedef struct {
    char *class_name;
    char *name;
    char *descriptor;
    variable_t *static_var; /* store static fields in the class */
} field_t;

typedef struct {
    u2 bootstrap_method_ref;
    u2 num_bootstrap_arguments;
    u2 *bootstrap_arguments;
} bootmethods_t;

typedef struct {
    u2 attribute_name_index;
    u4 attribute_length;
    u2 num_bootstrap_methods;
    bootmethods_t *bootstrap_methods;
} bootmethods_attr_t;

typedef struct class_file {
    constant_pool_t constant_pool;
    class_info_t *info;
    method_t *methods;
    field_t *fields;
    u2 fields_count;
    bootmethods_attr_t *bootstrap;
    bool initialized;
    struct class_file *next;
    struct class_file *prev;
} class_file_t;

typedef struct {
    class_file_t *clazz;
    char *name;
} meta_class_t;

class_header_t get_class_header(FILE *class_file);
class_info_t *get_class_info(FILE *class_file);
method_t *get_methods(FILE *class_file, constant_pool_t *cp);
void read_method_attributes(FILE *class_file,
                            method_info *info,
                            code_t *code,
                            constant_pool_t *cp);
uint16_t get_number_of_parameters(method_t *method);
field_t *find_field(const char *name, const char *desc, class_file_t *clazz);
method_t *find_method(const char *name, const char *desc, class_file_t *clazz);
method_t *find_method_from_index(uint16_t idx, class_file_t *clazz);
class_file_t get_class(FILE *class_file);
char *find_class_name_from_index(uint16_t idx, class_file_t *clazz);
CONSTANT_FieldOrMethodRef_info *get_fieldref(constant_pool_t *cp, u2 idx);
char *find_field_info_from_index(uint16_t idx,
                                 class_file_t *clazz,
                                 char **name_info,
                                 char **descriptor_info);
void read_field_attributes(FILE *class_file, field_info *info);
bootmethods_t *find_bootstrap_method(uint16_t idx, class_file_t *clazz);
bootmethods_attr_t *read_bootstrap_attribute(FILE *class_file,
                                             constant_pool_t *cp);
field_t *get_fields(FILE *class_file, constant_pool_t *cp, class_file_t *clazz);