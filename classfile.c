#include "classfile.h"

class_header_t get_class_header(FILE *class_file)
{
    return (class_header_t){
        .magic = read_u4(class_file),
        .major_version = read_u2(class_file),
        .minor_version = read_u2(class_file),
    };
}

class_info_t *get_class_info(FILE *class_file)
{
    class_info_t *info = malloc(sizeof(class_info_t));
    info->access_flags = read_u2(class_file);
    info->this_class = read_u2(class_file);
    info->super_class = read_u2(class_file);

    u2 interfaces_count = read_u2(class_file);
    assert(!interfaces_count && "This VM does not support interfaces.");
    return info;
}

/**
 * Get the number of parameters that a method takes.
 * Use the descriptor string of the method to determine its signature.
 */
uint16_t get_number_of_parameters(method_t *method)
{
    uint16_t num_param = 0;
    for (size_t i = 1; method->descriptor[i] != ')'; ++i) {
        /* if type is reference, skip class name */
        if (method->descriptor[i] == 'L') {
            while (method->descriptor[++i] != ';')
                ;
        }
        num_param++;
    }
    return num_param;
}

/**
 * Find the field with the given name and signature.
 * The difference between `find_field` and `object_heap:find_field_addr`
 * is that `find_field` is used to find class level field  (i.e. static field),
 * and `find_field_addr` is used to find object level field.
 *
 * @param name the field name
 * @param desc the field descriptor string, e.g. "(I)I"
 * @param clazz the parsed class file
 * @return the field if it was found, or NULL
 */
field_t *find_field(const char *name, const char *desc, class_file_t *clazz)
{
    field_t *field = clazz->fields;
    for (u2 i = 0; i < clazz->fields_count; ++i, field++) {
        if (!(strcmp(name, field->name) || strcmp(desc, field->descriptor)))
            return field;
    }
    return NULL;
}

/**
 * Find the method with the given name and signature.
 * The descriptor is necessary because Java allows method overloading.
 * This needs to be called directly to invoke main(),
 * or to find method from specific class.
 *
 * @param name the method name, e.g. "factorial"
 * @param desc the method descriptor string, e.g. "(I)I"
 * @param clazz the parsed class file
 * @return the method if it was found, or NULL
 */
method_t *find_method(const char *name, const char *desc, class_file_t *clazz)
{
    for (method_t *method = clazz->methods; method->name; method++) {
        if (!(strcmp(name, method->name) || strcmp(desc, method->descriptor)))
            return method;
    }
    return NULL;
}

/**
 * Find the method info corresponding to the given constant pool index.
 *
 * @param index the constant pool index of the Methodref to call
 * @param clazz the parsed class file
 * @param name_info the pointer that will contain method name on return
 * @param descriptor_info the pointer that will contain method information on
 * return
 * @return the class name that contains this method
 */
char *find_method_info_from_index(uint16_t idx,
                                  class_file_t *clazz,
                                  char **name_info,
                                  char **descriptor_info)
{
    CONSTANT_FieldOrMethodRef_info *method_ref =
        get_methodref(&clazz->constant_pool, idx);
    const_pool_info *name_and_type =
        get_constant(&clazz->constant_pool, method_ref->name_and_type_index);
    assert(name_and_type->tag == CONSTANT_NameAndType &&
           "Expected a NameAndType");
    const_pool_info *name = get_constant(
        &clazz->constant_pool,
        ((CONSTANT_NameAndType_info *) name_and_type->info)->name_index);
    assert(name->tag == CONSTANT_Utf8 && "Expected a UTF8");
    const_pool_info *descriptor = get_constant(
        &clazz->constant_pool,
        ((CONSTANT_NameAndType_info *) name_and_type->info)->descriptor_index);
    assert(descriptor->tag == CONSTANT_Utf8 && "Expected a UTF8");
    CONSTANT_Class_info *class_info =
        get_class_name(&clazz->constant_pool, method_ref->class_index);
    const_pool_info *class_name =
        get_constant(&clazz->constant_pool, class_info->string_index);
    *name_info = (char *) name->info;
    *descriptor_info = (char *) descriptor->info;

    return (char *) class_name->info;
}

CONSTANT_FieldOrMethodRef_info *get_fieldref(constant_pool_t *cp, u2 idx)
{
    const_pool_info *field = get_constant(cp, idx);
    assert(field->tag == CONSTANT_FieldRef && "Expected a FieldRef");
    return (CONSTANT_FieldOrMethodRef_info *) field->info;
}

/**
 * Find the field info corresponding to the given constant pool index.
 *
 * @param index the constant pool index of the Methodref to call
 * @param clazz the parsed class file
 * @param name_info the pointer that will contain method name on return
 * @param descriptor_info the pointer that will contain method infomation on
 * return
 * @return the class name which has this field
 */
char *find_field_info_from_index(uint16_t idx,
                                 class_file_t *clazz,
                                 char **name_info,
                                 char **descriptor_info)
{
    CONSTANT_FieldOrMethodRef_info *field_ref =
        get_fieldref(&clazz->constant_pool, idx);
    const_pool_info *name_and_type =
        get_constant(&clazz->constant_pool, field_ref->name_and_type_index);
    assert(name_and_type->tag == CONSTANT_NameAndType &&
           "Expected a NameAndType");
    const_pool_info *name = get_constant(
        &clazz->constant_pool,
        ((CONSTANT_NameAndType_info *) name_and_type->info)->name_index);
    assert(name->tag == CONSTANT_Utf8 && "Expected a UTF8");
    const_pool_info *descriptor = get_constant(
        &clazz->constant_pool,
        ((CONSTANT_NameAndType_info *) name_and_type->info)->descriptor_index);
    assert(descriptor->tag == CONSTANT_Utf8 && "Expected a UTF8");
    CONSTANT_Class_info *class_info =
        get_class_name(&clazz->constant_pool, field_ref->class_index);
    const_pool_info *class_name =
        get_constant(&clazz->constant_pool, class_info->string_index);
    *name_info = (char *) name->info;
    *descriptor_info = (char *) descriptor->info;

    return (char *) class_name->info;
}

char *find_class_name_from_index(uint16_t idx, class_file_t *clazz)
{
    CONSTANT_Class_info *class = get_class_name(&clazz->constant_pool, idx);

    const_pool_info *name =
        get_constant(&clazz->constant_pool, class->string_index);
    assert(name->tag == CONSTANT_Utf8 && "Expected a UTF8");
    return (char *) name->info;
}

bootmethods_t *find_bootstrap_method(uint16_t idx, class_file_t *clazz)
{
    const_pool_info *info = get_constant(&clazz->constant_pool, idx);
    assert(info->tag == CONSTANT_InvokeDynamic && "Expected a InvokeDynanmic");
    return &clazz->bootstrap
                ->bootstrap_methods[((CONSTANT_InvokeDynamic_info *) info->info)
                                        ->bootstrap_method_attr_index];
}

void read_field_attributes(FILE *class_file, field_info *info)
{
    for (u2 i = 0; i < info->attributes_count; i++) {
        attribute_info ainfo = {
            .attribute_name_index = read_u2(class_file),
            .attribute_length = read_u4(class_file),
        };
        long attribute_end = ftell(class_file) + ainfo.attribute_length;
        /* Skip all the attribute */
        fseek(class_file, attribute_end, SEEK_SET);
    }
}

void read_method_attributes(FILE *class_file,
                            method_info *info,
                            code_t *code,
                            constant_pool_t *cp)
{
    bool found_code = false;
    for (u2 i = 0; i < info->attributes_count; i++) {
        attribute_info ainfo = {
            .attribute_name_index = read_u2(class_file),
            .attribute_length = read_u4(class_file),
        };
        long attribute_end = ftell(class_file) + ainfo.attribute_length;
        const_pool_info *type_constant =
            get_constant(cp, ainfo.attribute_name_index);
        assert(type_constant->tag == CONSTANT_Utf8 && "Expected a UTF8");
        if (!strcmp((char *) type_constant->info, "Code")) {
            assert(!found_code && "Duplicate method code");
            found_code = true;

            code->max_stack = read_u2(class_file);
            code->max_locals = read_u2(class_file);
            code->code_length = read_u4(class_file);
            code->code = malloc(code->code_length);
            assert(code->code && "Failed to allocate method code");
            size_t bytes_read =
                fread(code->code, 1, code->code_length, class_file);
            assert(bytes_read == code->code_length &&
                   "Failed to read method code");
        }
        /* Skip the rest of the attribute */
        fseek(class_file, attribute_end, SEEK_SET);
    }
    assert(found_code && "Missing method code");
}

bootmethods_attr_t *read_bootstrap_attribute(FILE *class_file,
                                             constant_pool_t *cp)
{
    u2 attributes_count = read_u2(class_file);
    for (u2 i = 0; i < attributes_count; i++) {
        attribute_info ainfo = {
            .attribute_name_index = read_u2(class_file),
            .attribute_length = read_u4(class_file),
        };
        long attribute_end = ftell(class_file) + ainfo.attribute_length;
        const_pool_info *type_constant =
            get_constant(cp, ainfo.attribute_name_index);
        assert(type_constant->tag == CONSTANT_Utf8 && "Expected a UTF8");
        if (!strcmp((char *) type_constant->info, "BootstrapMethods")) {
            bootmethods_attr_t *bootstrap = malloc(sizeof(*bootstrap));

            bootstrap->num_bootstrap_methods = read_u2(class_file);
            bootstrap->bootstrap_methods = malloc(
                sizeof(bootmethods_t) * bootstrap->num_bootstrap_methods);

            assert(bootstrap->bootstrap_methods &&
                   "Failed to allocate bootstrap method");
            for (int j = 0; j < bootstrap->num_bootstrap_methods; ++j) {
                bootstrap->bootstrap_methods[j].bootstrap_method_ref =
                    read_u2(class_file);
                bootstrap->bootstrap_methods[j].num_bootstrap_arguments =
                    read_u2(class_file);
                bootstrap->bootstrap_methods[j].bootstrap_arguments = malloc(
                    sizeof(u2) *
                    bootstrap->bootstrap_methods[j].num_bootstrap_arguments);
                assert(bootstrap->bootstrap_methods[j].bootstrap_arguments &&
                       "Failed to allocate bootstrap argument");
                for (int k = 0;
                     k <
                     bootstrap->bootstrap_methods[j].num_bootstrap_arguments;
                     ++k) {
                    bootstrap->bootstrap_methods[j].bootstrap_arguments[k] =
                        read_u2(class_file);
                }
            }
            return bootstrap;
        }
        /* Skip the rest of the attribute */
        fseek(class_file, attribute_end, SEEK_SET);
    }
    return NULL;
}

#define IS_STATIC 0x0008

field_t *get_fields(FILE *class_file, constant_pool_t *cp, class_file_t *clazz)
{
    u2 fields_count = read_u2(class_file);
    clazz->fields_count = fields_count;
    field_t *fields = malloc(sizeof(*fields) * (fields_count + 1));
    assert(fields && "Failed to allocate methods");

    field_t *field = fields;
    for (u2 i = 0; i < fields_count; i++, field++) {
        field_info info = {
            .access_flags = read_u2(class_file),
            .name_index = read_u2(class_file),
            .descriptor_index = read_u2(class_file),
            .attributes_count = read_u2(class_file),
        };

        const_pool_info *name = get_constant(cp, info.name_index);
        assert(name->tag == CONSTANT_Utf8 && "Expected a UTF8");
        field->name = (char *) name->info;
        const_pool_info *descriptor = get_constant(cp, info.descriptor_index);
        assert(descriptor->tag == CONSTANT_Utf8 && "Expected a UTF8");
        field->descriptor = (char *) descriptor->info;
        field->static_var = malloc(sizeof(variable_t));

        read_field_attributes(class_file, &info);
    }

    /* Mark end of array with NULL name */
    field->name = NULL;
    return fields;
}

method_t *get_methods(FILE *class_file, constant_pool_t *cp)
{
    u2 method_count = read_u2(class_file);
    method_t *methods = malloc(sizeof(*methods) * (method_count + 1));
    assert(methods && "Failed to allocate methods");

    method_t *method = methods;
    for (u2 i = 0; i < method_count; i++, method++) {
        method_info info = {
            .access_flags = read_u2(class_file),
            .name_index = read_u2(class_file),
            .descriptor_index = read_u2(class_file),
            .attributes_count = read_u2(class_file),
        };

        const_pool_info *name = get_constant(cp, info.name_index);
        assert(name->tag == CONSTANT_Utf8 && "Expected a UTF8");
        method->name = (char *) name->info;
        const_pool_info *descriptor = get_constant(cp, info.descriptor_index);
        assert(descriptor->tag == CONSTANT_Utf8 && "Expected a UTF8");
        method->descriptor = (char *) descriptor->info;

        read_method_attributes(class_file, &info, &method->code, cp);
    }

    /* Mark end of array with NULL name */
    method->name = NULL;
    return methods;
}

/**
 * Read an entire class file.
 * The end of the parsed methods array is marked by a method with a NULL name.
 *
 * @param class_file the open file to read
 * @return the parsed class file
 */
class_file_t get_class(FILE *class_file)
{
    /* Read the leading header of the class file */
    get_class_header(class_file);

    /* Read the constant pool */
    class_file_t clazz = {.constant_pool = get_constant_pool(class_file)};

    /* Read information about the class that was compiled. */
    clazz.info = get_class_info(class_file);

    /* Read the list of fields */
    clazz.fields = get_fields(class_file, &clazz.constant_pool, &clazz);

    /* Read the list of static methods */
    clazz.methods = get_methods(class_file, &clazz.constant_pool);

    /* Read the list of attributes */
    clazz.bootstrap =
        read_bootstrap_attribute(class_file, &clazz.constant_pool);

    clazz.initialized = false;

    return clazz;
}