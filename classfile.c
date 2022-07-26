#include "classfile.h"

class_header_t get_class_header(FILE *class_file)
{
    return (class_header_t){
        .magic = read_u4(class_file),
        .major_version = read_u2(class_file),
        .minor_version = read_u2(class_file),
    };
}

class_info_t get_class_info(FILE *class_file)
{
    class_info_t info = {
        .access_flags = read_u2(class_file),
        .this_class = read_u2(class_file),
        .super_class = read_u2(class_file),
    };
    u2 interfaces_count = read_u2(class_file);
    assert(!interfaces_count && "This VM does not support interfaces.");
    u2 fields_count = read_u2(class_file);
    assert(!fields_count && "This VM does not support fields.");
    return info;
}

/**
 * Get the number of integer parameters that a method takes.
 * Use the descriptor string of the method to determine its signature.
 */
uint16_t get_number_of_parameters(method_t *method)
{
    /* Type descriptors have the length ( + #params + ) + return type */
    return strlen(method->descriptor) - 3;
}

/**
 * Find the method with the given name and signature.
 * The descriptor is necessary because Java allows method overloading.
 * This only needs to be called directly to invoke main();
 * for the invokestatic instruction, use find_method_from_index().
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
 * Find the method corresponding to the given constant pool index.
 *
 * @param index the constant pool index of the Methodref to call
 * @param clazz the parsed class file
 * @return the method if it was found, or NULL
 */
method_t *find_method_from_index(uint16_t idx, class_file_t *clazz)
{
    CONSTANT_NameAndType_info *name_and_type =
        get_method_name_and_type(&clazz->constant_pool, idx);
    const_pool_info *name =
        get_constant(&clazz->constant_pool, name_and_type->name_index);
    assert(name->tag == CONSTANT_Utf8 && "Expected a UTF8");
    const_pool_info *descriptor =
        get_constant(&clazz->constant_pool, name_and_type->descriptor_index);
    assert(descriptor->tag == CONSTANT_Utf8 && "Expected a UTF8");
    return find_method((char *) name->info, (char *) descriptor->info, clazz);
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

#define IS_STATIC 0x0008

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

        /* FIXME: this VM can only execute static methods, while every class
         * has a constructor method <init>
         */
        if (strcmp(method->name, "<init>"))
            assert((info.access_flags & IS_STATIC) &&
                   "Only static methods are supported by this VM.");

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
    get_class_info(class_file);

    /* Read the list of static methods */
    clazz.methods = get_methods(class_file, &clazz.constant_pool);
    return clazz;
}

/**
 * Frees the memory used by a parsed class file.
 *
 * @param class the parsed class file
 */
void free_class(class_file_t *clazz)
{
    constant_pool_t *cp = &clazz->constant_pool;
    for (u2 i = 0; i < cp->constant_pool_count; i++)
        free(cp->constant_pool[i].info);
    free(cp->constant_pool);

    for (method_t *method = clazz->methods; method->name; method++)
        free(method->code.code);
    free(clazz->methods);
}