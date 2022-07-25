#include "constant-pool.h"

/* Read unsigned big-endian integers */
u1 read_u1(FILE *class_file)
{
    int result = fgetc(class_file);
    assert(result != EOF && "Reached end of file prematurely");
    return result;
}

u2 read_u2(FILE *class_file)
{
    return (u2) read_u1(class_file) << 8 | read_u1(class_file);
}

u4 read_u4(FILE *class_file)
{
    return (u4) read_u2(class_file) << 16 | read_u2(class_file);
}

/**
 * Get the constant at the given index in a constant pool.
 * Assert that the index is valid (i.e. between 1 and the pool size).
 *
 * @param constant_pool the class's constant pool
 * @param index the 1-indexed constant pool index
 * @return the constant at the given index
 */
const_pool_info *get_constant(constant_pool_t *constant_pool, u2 index)
{
    assert(0 < index && index <= constant_pool->constant_pool_count &&
           "Invalid constant pool index");
    /* Convert 1-indexed index to 0-indexed index */
    return &constant_pool->constant_pool[index - 1];
}

CONSTANT_NameAndType_info *get_method_name_and_type(constant_pool_t *cp, u2 idx)
{
    const_pool_info *method = get_constant(cp, idx);
    assert(method->tag == CONSTANT_MethodRef && "Expected a MethodRef");
    const_pool_info *name_and_type_constant = get_constant(
        cp,
        ((CONSTANT_FieldOrMethodRef_info *) method->info)->name_and_type_index);
    assert(name_and_type_constant->tag == CONSTANT_NameAndType &&
           "Expected a NameAndType");
    return (CONSTANT_NameAndType_info *) name_and_type_constant->info;
}

constant_pool_t get_constant_pool(FILE *class_file)
{
    constant_pool_t cp = {
        /* Constant pool count includes unused constant at index 0 */
        .constant_pool_count = read_u2(class_file) - 1,
        .constant_pool =
            malloc(sizeof(const_pool_info) * cp.constant_pool_count),
    };
    assert(cp.constant_pool && "Failed to allocate constant pool");

    const_pool_info *constant = cp.constant_pool;
    for (u2 i = 0; i < cp.constant_pool_count; i++, constant++) {
        constant->tag = read_u1(class_file);
        switch (constant->tag) {
        case CONSTANT_Utf8: {
            u2 length = read_u2(class_file);
            char *value = malloc(length + 1);
            assert(value && "Failed to allocate UTF8 constant");
            size_t bytes_read = fread(value, 1, length, class_file);
            assert(bytes_read == length && "Failed to read UTF8 constant");
            value[length] = '\0';
            constant->info = (u1 *) value;
            break;
        }

        case CONSTANT_Integer: {
            CONSTANT_Integer_info *value = malloc(sizeof(*value));
            assert(value && "Failed to allocate integer constant");
            value->bytes = read_u4(class_file);
            constant->info = (u1 *) value;
            break;
        }

        case CONSTANT_Class: {
            CONSTANT_Class_info *value = malloc(sizeof(*value));
            assert(value && "Failed to allocate class constant");
            value->string_index = read_u2(class_file);
            constant->info = (u1 *) value;
            break;
        }

        case CONSTANT_MethodRef:
        case CONSTANT_FieldRef: {
            CONSTANT_FieldOrMethodRef_info *value = malloc(sizeof(*value));
            assert(value &&
                   "Failed to allocate FieldRef or MethodRef constant");
            value->class_index = read_u2(class_file);
            value->name_and_type_index = read_u2(class_file);
            constant->info = (u1 *) value;
            break;
        }

        case CONSTANT_NameAndType: {
            CONSTANT_NameAndType_info *value = malloc(sizeof(*value));
            assert(value && "Failed to allocate NameAndType constant");
            value->name_index = read_u2(class_file);
            value->descriptor_index = read_u2(class_file);
            constant->info = (u1 *) value;
            break;
        }

        default:
            fprintf(stderr, "Unknown constant type %d\n", constant->tag);
            exit(1);
        }
    }
    return cp;
}
