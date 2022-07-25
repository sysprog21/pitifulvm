/* PitifulVM is a minimalist Java Virtual Machine implementation written in C.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classfile.h"
#include "constant-pool.h"
#include "stack.h"

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
 * @return stack_entry that contain the method return value and its type. Is a
 *         heap-allocated pointer which should be free from the caller
 *
 */
stack_entry_t *execute(method_t *method,
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
            stack_entry_t *ret = malloc(sizeof(stack_entry_t));
            ret->entry.int_value = (int32_t) pop_int(op_stack);
            ret->type = STACK_ENTRY_INT;

            free(op_stack->store);
            free(op_stack);

            return ret;
        }

        /* Return void from method */
        case i_return: {
            stack_entry_t *ret = malloc(sizeof(stack_entry_t));
            ret->type = STACK_ENTRY_NONE;

            free(op_stack->store);
            free(op_stack);

            return ret;
        }

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

            stack_entry_t *exec_res = execute(own_method, own_locals, clazz);
            switch (exec_res->type) {
            case STACK_ENTRY_INT:
                push_int(op_stack, exec_res->entry.int_value);
                break;
            case STACK_ENTRY_NONE:
                /* nothing */
                break;
            default:
                assert(0 && "unknown return type");
            }

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

        default:
            fprintf(stderr, "Unknown instruction %x\n", current);
            exit(1);
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
    stack_entry_t *result = execute(main_method, locals, &clazz);
    assert(result->type == STACK_ENTRY_NONE && "main() should return void");
    free(result);

    free_class(&clazz);

    return 0;
}
