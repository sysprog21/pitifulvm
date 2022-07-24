/* PitifulVM is a minimalist Java Virtual Machine implementation written in C.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stack.h"

typedef struct {
    u4 magic;
    u2 minor_version, major_version;
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

typedef enum {
    CONSTANT_Utf8 = 1,
    CONSTANT_Integer = 3,
    CONSTANT_Class = 7,
    CONSTANT_FieldRef = 9,
    CONSTANT_MethodRef = 10,
    CONSTANT_NameAndType = 12,
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
    u2 name_index;
    u2 descriptor_index;
} CONSTANT_NameAndType_info;

typedef struct {
    const_pool_tag_t tag;
    u1 *info;
} const_pool_info;

typedef struct {
    u2 constant_pool_count;
    const_pool_info *constant_pool;
} constant_pool_t;

typedef struct {
    constant_pool_t constant_pool;
    method_t *methods;
} class_file_t;

typedef enum {
    i_iconst_m1 = 0x2,
    i_iconst_0 = 0x3,
    i_iconst_1 = 0x4,
    i_iconst_2 = 0x5,
    i_iconst_3 = 0x6,
    i_iconst_4 = 0x7,
    i_iconst_5 = 0x8,
    i_bipush = 0x10,
    i_sipush = 0x11,
    i_ldc = 0x12,
    i_iload = 0x15,
    i_iload_0 = 0x1a,
    i_iload_1 = 0x1b,
    i_iload_2 = 0x1c,
    i_iload_3 = 0x1d,
    i_istore = 0x36,
    i_istore_0 = 0x3b,
    i_istore_1 = 0x3c,
    i_istore_2 = 0x3d,
    i_istore_3 = 0x3e,
    i_iadd = 0x60,
    i_isub = 0x64,
    i_imul = 0x68,
    i_idiv = 0x6c,
    i_irem = 0x70,
    i_ineg = 0x74,
    i_iinc = 0x84,
    i_ifeq = 0x99,
    i_ifne = 0x9a,
    i_iflt = 0x9b,
    i_ifge = 0x9c,
    i_ifgt = 0x9d,
    i_ifle = 0x9e,
    i_if_icmpeq = 0x9f,
    i_if_icmpne = 0xa0,
    i_if_icmplt = 0xa1,
    i_if_icmpge = 0xa2,
    i_if_icmpgt = 0xa3,
    i_if_icmple = 0xa4,
    i_goto = 0xa7,
    i_ireturn = 0xac,
    i_return = 0xb1,
    i_getstatic = 0xb2,
    i_invokevirtual = 0xb6,
    i_invokestatic = 0xb8,
} jvm_opcode_t;

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

class_header_t get_class_header(FILE *class_file)
{
    return (class_header_t){
        .magic = read_u4(class_file),
        .major_version = read_u2(class_file),
        .minor_version = read_u2(class_file),
    };
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

static inline void bipush(stack_frame_t *op_stack,
                          uint32_t pc,
                          uint8_t *code_buf)
{
    int8_t param = code_buf[pc + 1];

    push_byte(op_stack, param);
}

static inline void sipush(stack_frame_t *op_stack,
                          uint32_t pc,
                          uint8_t *code_buf)
{
    uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
    int16_t res = ((param1 << 8) | param2);

    push_short(op_stack, res);
}

static inline void iadd(stack_frame_t *op_stack)
{
    int32_t op1 = pop_int(op_stack);
    int32_t op2 = pop_int(op_stack);

    push_int(op_stack, op1 + op2);
}

static inline void isub(stack_frame_t *op_stack)
{
    int32_t op1 = pop_int(op_stack);
    int32_t op2 = pop_int(op_stack);

    push_int(op_stack, op2 - op1);
}

static inline void imul(stack_frame_t *op_stack)
{
    int32_t op1 = pop_int(op_stack);
    int32_t op2 = pop_int(op_stack);

    push_int(op_stack, op1 * op2);
}

static inline void idiv(stack_frame_t *op_stack)
{
    int32_t op1 = pop_int(op_stack);
    int32_t op2 = pop_int(op_stack);

    push_int(op_stack, op2 / op1);
}

static inline void irem(stack_frame_t *op_stack)
{
    int32_t op1 = pop_int(op_stack);
    int32_t op2 = pop_int(op_stack);

    push_int(op_stack, op2 % op1);
}

static inline void ineg(stack_frame_t *op_stack)
{
    int32_t op1 = pop_int(op_stack);

    push_int(op_stack, -op1);
}

static inline void invokevirtual(stack_frame_t *op_stack)
{
    int32_t op = pop_int(op_stack);

    /* FIXME: the implement is not correct. */
    printf("%d\n", op);
}

static inline void iconst(stack_frame_t *op_stack, uint8_t current)
{
    push_int(op_stack, current - i_iconst_0);
}

/**
 * Execute the opcode instructions of a method until it returns.
 *
 * @param method the method to run
 * @param locals the array of local variables, including the method parameters.
 *               Except for parameters, the locals are uninitialized.
 * @param clazz the class file the method belongs to
 * @return if the method returns an int, a heap-allocated pointer to it;
 *         NULL if the method returns void, NULL;
 */
int32_t *execute(method_t *method,
                 local_variable_t *locals,
                 class_file_t *clazz)
{
    code_t code = method->code;
    stack_frame_t *op_stack = malloc(sizeof(stack_frame_t));
    init_stack(op_stack, code.max_stack);

    /* position at the program to be run */
    uint32_t pc = 0;
    uint8_t *code_buf = code.code;

    int loop_count = 0;
    while (pc < code.code_length) {
        loop_count += 1;
        uint8_t current = code_buf[pc];

        /* Reference:
         * https://en.wikipedia.org/wiki/Java_bytecode_instruction_listings
         */
        switch (current) {
        /* Return int from method */
        case i_ireturn: {
            int32_t *ret = malloc(sizeof(int32_t));
            *ret = pop_int(op_stack);
            free(op_stack->store);
            free(op_stack);
            return ret;
        }

        /* Return void from method */
        case i_return:
            free(op_stack->store);
            free(op_stack);
            return NULL;

        /* Invoke a class (static) method */
        case i_invokestatic: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            uint16_t index = ((param1 << 8) | param2);

            /* the method to be called */
            method_t *own_method = find_method_from_index(index, clazz);
            uint16_t num_params = get_number_of_parameters(own_method);
            local_variable_t own_locals[own_method->code.max_locals];
            for (int i = num_params - 1; i >= 0; i--)
                pop_to_local(op_stack, &own_locals[i]);

            int32_t *exec_res = execute(own_method, own_locals, clazz);
            if (exec_res)
                push_int(op_stack, *exec_res);

            free(exec_res);
            pc += 3;
            break;
        }

        /* Branch if int comparison with zero succeeds: if equals */
        case i_ifeq: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            int32_t conditional = pop_int(op_stack);
            pc += 3;
            if (conditional == 0) {
                int16_t res = ((param1 << 8) | param2);
                pc += res - 3;
            }
            break;
        }

        /* Branch if int comparison with zero succeeds: if not equals */
        case i_ifne: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            int32_t conditional = pop_int(op_stack);
            pc += 3;
            if (conditional != 0) {
                int16_t res = ((param1 << 8) | param2);
                pc += res - 3;
            }
            break;
        }

        /* Branch if int comparison with zero succeeds: if less than 0 */
        case i_iflt: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            int32_t conditional = pop_int(op_stack);
            pc += 3;
            if (conditional < 0) {
                int16_t res = ((param1 << 8) | param2);
                pc += res - 3;
            }
            break;
        }

        /* Branch if int comparison with zero succeeds: if >= 0 */
        case i_ifge: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            int32_t conditional = pop_int(op_stack);
            pc += 3;
            if (conditional >= 0) {
                int16_t res = ((param1 << 8) | param2);
                pc += res - 3;
            }
            break;
        }

        /* Branch if int comparison with zero succeeds: if greater than 0 */
        case i_ifgt: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            int32_t conditional = pop_int(op_stack);
            pc += 3;
            if (conditional > 0) {
                int16_t res = ((param1 << 8) | param2);
                pc += res - 3;
            }
            break;
        }

        /* Branch if int comparison with zero succeeds: if <= 0 */
        case i_ifle: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            int32_t conditional = pop_int(op_stack);
            pc += 3;
            if (conditional <= 0) {
                int16_t res = ((param1 << 8) | param2);
                pc += res - 3;
            }
            break;
        }

        /* Branch if int comparison succeeds: if equals */
        case i_if_icmpeq: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            int32_t op1 = pop_int(op_stack), op2 = pop_int(op_stack);
            pc += 3;
            if (op2 == op1) {
                int16_t res = ((param1 << 8) | param2);
                pc += res - 3;
            }
            break;
        }

        /* Branch if int comparison succeeds: if not equals */
        case i_if_icmpne: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            int32_t op1 = pop_int(op_stack), op2 = pop_int(op_stack);
            pc += 3;
            if (op2 != op1) {
                int16_t res = ((param1 << 8) | param2);
                pc += res - 3;
            }
            break;
        }

        /* Branch if int comparison succeeds: if less than */
        case i_if_icmplt: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            int32_t op1 = pop_int(op_stack), op2 = pop_int(op_stack);
            pc += 3;
            if (op2 < op1) {
                int16_t res = ((param1 << 8) | param2);
                pc += res - 3;
            }
            break;
        }

        /* Branch if int comparison succeeds: if greater than or equal to */
        case i_if_icmpge: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            int32_t op1 = pop_int(op_stack), op2 = pop_int(op_stack);
            pc += 3;
            if (op2 >= op1) {
                int16_t res = ((param1 << 8) | param2);
                pc += res - 3;
            }
            break;
        }

        /* Branch if int comparison succeeds: if greater than */
        case i_if_icmpgt: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            int32_t op1 = pop_int(op_stack), op2 = pop_int(op_stack);
            pc += 3;
            if (op2 > op1) {
                int16_t res = ((param1 << 8) | param2);
                pc += res - 3;
            }
            break;
        }

        /* Branch if int comparison succeeds: if less than or equal to */
        case i_if_icmple: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            int32_t op1 = pop_int(op_stack), op2 = pop_int(op_stack);
            pc += 3;
            if (op2 <= op1) {
                int16_t res = ((param1 << 8) | param2);
                pc += res - 3;
            }
            break;
        }

        /* Branch always */
        case i_goto: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            int16_t res = ((param1 << 8) | param2);
            pc += res;
            break;
        }

        /* Push item from run-time constant pool */
        case i_ldc: {
            constant_pool_t constant_pool = clazz->constant_pool;

            /* find the parameter which will be the index from which we retrieve
             * constant in the constant pool.
             */
            int16_t param = code_buf[pc + 1];

            /* get the constant */
            uint8_t *info = get_constant(&constant_pool, param)->info;

            /* need to check type */
            push_int(op_stack, ((CONSTANT_Integer_info *) info)->bytes);
            pc += 2;
            break;
        }

        /* Load int from local variable */
        case i_iload_0:
        case i_iload_1:
        case i_iload_2:
        case i_iload_3: {
            int32_t param = current - i_iload_0;
            int32_t loaded;

            loaded = locals[param].entry.int_value;
            push_int(op_stack, loaded);
            pc += 1;
            break;
        }

        /* Load int from local variable */
        case i_iload: {
            int32_t param = code_buf[pc + 1];
            int32_t loaded;

            loaded = locals[param].entry.int_value;
            push_int(op_stack, loaded);

            pc += 2;
            break;
        }

        /* Store int into local variable */
        case i_istore: {
            int32_t param = code_buf[pc + 1];
            int32_t stored = pop_int(op_stack);
            locals[param].entry.int_value = stored;
            locals[param].type = STACK_ENTRY_INT;
            pc += 2;
            break;
        }

        /* Store int into local variable */
        case i_istore_0:
        case i_istore_1:
        case i_istore_2:
        case i_istore_3: {
            int32_t param = current - i_istore_0;
            int32_t stored = pop_int(op_stack);
            locals[param].entry.int_value = stored;
            locals[param].type = STACK_ENTRY_INT;
            pc += 1;
            break;
        }

        /* Increment local variable by constant */
        case i_iinc: {
            uint8_t i = code_buf[pc + 1];
            int8_t b = code_buf[pc + 2]; /* signed value */
            locals[i].entry.int_value += b;
            pc += 3;
            break;
        }

        /* Push byte */
        case i_bipush:
            bipush(op_stack, pc, code_buf);
            pc += 2;
            break;

        /* Add int */
        case i_iadd:
            iadd(op_stack);
            pc += 1;
            break;

        /* Subtract int */
        case i_isub:
            isub(op_stack);
            pc += 1;
            break;

        /* Multiply int */
        case i_imul:
            imul(op_stack);
            pc += 1;
            break;

        /* Divide int */
        case i_idiv:
            idiv(op_stack);
            pc += 1;
            break;

        /* Remainder int */
        case i_irem:
            irem(op_stack);
            pc += 1;
            break;

            /* Negate int */
        case i_ineg:
            ineg(op_stack);
            pc += 1;
            break;

        /* Get static field from class */
        case i_getstatic:
            /* FIXME: unimplemented */
            pc += 3;
            break;

        /* Invoke instance method; dispatch based on class */
        case i_invokevirtual:
            invokevirtual(op_stack);
            pc += 3;
            break;

        /* Push int constant */
        case i_iconst_m1:
        case i_iconst_0:
        case i_iconst_1:
        case i_iconst_2:
        case i_iconst_3:
        case i_iconst_4:
        case i_iconst_5:
            iconst(op_stack, current);
            pc += 1;
            break;

        /* Push short */
        case i_sipush:
            sipush(op_stack, pc, code_buf);
            pc += 3;
            break;
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        return -1;

    /* attempt to read given class file */
    FILE *class_file = fopen(argv[1], "r");
    assert(class_file && "Failed to open file");

    /* parse the class file */
    class_file_t clazz = get_class(class_file);
    int error = fclose(class_file);
    assert(!error && "Failed to close file");

    /* execute the main method if found */
    method_t *main_method =
        find_method("main", "([Ljava/lang/String;)V", &clazz);
    assert(main_method && "Missing main() method");

    /* FIXME: locals[0] contains a reference to String[] args, but right now
     * we lack of the support for java.lang.Object. Leave it uninitialized.
     */
    local_variable_t locals[main_method->code.max_locals];
    int32_t *result = execute(main_method, locals, &clazz);
    assert(!result && "main() should return void");

    free_class(&clazz);

    return 0;
}
