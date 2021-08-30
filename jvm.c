/* PitifulVM is a minimalist Java Virtual Machine implementation written in C.
 */

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "class-heap.h"
#include "classfile.h"
#include "constant-pool.h"
#include "object-heap.h"
#include "stack.h"

typedef enum {
    i_iconst_m1 = 0x2,
    i_iconst_0 = 0x3,
    i_iconst_1 = 0x4,
    i_iconst_2 = 0x5,
    i_iconst_3 = 0x6,
    i_iconst_4 = 0x7,
    i_iconst_5 = 0x8,
    i_lconst_0 = 0x9,
    i_lconst_1 = 0xa,
    i_bipush = 0x10,
    i_sipush = 0x11,
    i_ldc = 0x12,
    i_ldc2_w = 0x14,
    i_iload = 0x15,
    i_lload = 0x16,
    i_aload = 0x19,
    i_iload_0 = 0x1a,
    i_iload_1 = 0x1b,
    i_iload_2 = 0x1c,
    i_iload_3 = 0x1d,
    i_lload_0 = 0x1e,
    i_lload_1 = 0x1f,
    i_lload_2 = 0x20,
    i_lload_3 = 0x21,
    i_aload_0 = 0x2a,
    i_aload_1 = 0x2b,
    i_aload_2 = 0x2c,
    i_aload_3 = 0x2d,
    i_istore = 0x36,
    i_lstore = 0x37,
    i_astore = 0x3a,
    i_istore_0 = 0x3b,
    i_istore_1 = 0x3c,
    i_istore_2 = 0x3d,
    i_istore_3 = 0x3e,
    i_lstore_0 = 0x3f,
    i_lstore_1 = 0x40,
    i_lstore_2 = 0x41,
    i_lstore_3 = 0x42,
    i_astore_0 = 0x4b,
    i_astore_1 = 0x4c,
    i_astore_2 = 0x4d,
    i_astore_3 = 0x4e,
    i_pop = 0x57,
    i_dup = 0x59,
    i_iadd = 0x60,
    i_ladd = 0x61,
    i_isub = 0x64,
    i_lsub = 0x65,
    i_imul = 0x68,
    i_lmul = 0x69,
    i_idiv = 0x6c,
    i_ldiv = 0x6d,
    i_irem = 0x70,
    i_lrem = 0x71,
    i_ineg = 0x74,
    i_iinc = 0x84,
    i_i2l = 0x85,
    i_l2i = 0x88,
    i_lcmp = 0x94,
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
    i_lreturn = 0xad,
    i_areturn = 0xb0,
    i_return = 0xb1,
    i_getstatic = 0xb2,
    i_putstatic = 0xb3,
    i_getfield = 0xb4,
    i_putfield = 0xb5,
    i_invokevirtual = 0xb6,
    i_invokespecial = 0xb7,
    i_invokestatic = 0xb8,
    i_new = 0xbb,
} jvm_opcode_t;

/* TODO: add -cp arg to achieve class path select */
static char *prefix = NULL;

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
    int64_t op = pop_int(op_stack);

    /* FIXME: the implement is not correct. */
    printf("%" PRId64 "\n", op);
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

        /* Return long from method */
        case i_lreturn: {
            stack_entry_t *ret = malloc(sizeof(stack_entry_t));
            ret->entry.long_value = (int64_t) pop_int(op_stack);
            ret->type = STACK_ENTRY_LONG;

            free(op_stack->store);
            free(op_stack);

            return ret;
        }

        /* Return long from method */
        case i_areturn: {
            stack_entry_t *ret = malloc(sizeof(stack_entry_t));
            ret->entry.ptr_value = pop_ref(op_stack);
            ret->type = STACK_ENTRY_REF;

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
            char *method_name, *method_descriptor, *class_name;
            class_name = find_method_info_from_index(index, clazz, &method_name,
                                                     &method_descriptor);

            class_file_t *target_class =
                find_or_add_class_to_heap(class_name, prefix);
            assert(target_class && "Failed to load class in invokestatic");

            method_t *own_method =
                find_method(method_name, method_descriptor, target_class);
            uint16_t num_params = get_number_of_parameters(own_method);
            local_variable_t own_locals[own_method->code.max_locals];
            for (int i = num_params - 1; i >= 0; i--)
                pop_to_local(op_stack, &own_locals[i]);

            stack_entry_t *exec_res =
                execute(own_method, own_locals, target_class);
            switch (exec_res->type) {
            case STACK_ENTRY_INT:
                push_int(op_stack, exec_res->entry.int_value);
                break;
            case STACK_ENTRY_LONG:
                push_long(op_stack, exec_res->entry.long_value);
                break;
            case STACK_ENTRY_REF:
                push_ref(op_stack, exec_res->entry.ptr_value);
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

        /* Compare long */
        case i_lcmp: {
            int64_t op1 = pop_int(op_stack), op2 = pop_int(op_stack);
            if (op1 < op2) {
                push_int(op_stack, 1);
            } else if (op1 == op2) {
                push_int(op_stack, 0);
            } else {
                push_int(op_stack, -1);
            }
            pc += 1;
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

        /* Push long or double from run-time constant pool (wide index) */
        case i_ldc2_w: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            uint16_t index = ((param1 << 8) | param2);

            uint64_t high = ((CONSTANT_LongOrDouble_info *) get_constant(
                                 &clazz->constant_pool, index)
                                 ->info)
                                ->high_bytes;
            uint64_t low = ((CONSTANT_LongOrDouble_info *) get_constant(
                                &clazz->constant_pool, index)
                                ->info)
                               ->low_bytes;
            int64_t value = high << 32 | low;
            push_long(op_stack, value);
            pc += 3;
            break;
        }

        /* Load long from local variable */
        case i_lload: {
            int32_t param = code_buf[pc + 1];
            int64_t loaded;
            loaded = locals[param].entry.long_value;
            push_long(op_stack, loaded);

            pc += 2;
            break;
        }

        /* FIXME: this implementation has some bugs.
         * In standard JVM, one stack entry only store four
         * bytes data, so in some method descriptor (e.g (JJ)V)
         * the long value will store in locals[0] and locals[2]
         * rather than locals[0] and locals[1].
         */
        /* Load long from local variable */
        case i_lload_0:
        case i_lload_1:
        case i_lload_2:
        case i_lload_3: {
            int64_t param = current - i_lload_0;
            int64_t loaded;
            loaded = locals[param].entry.long_value;
            push_long(op_stack, loaded);

            pc += 1;
            break;
        }

        /* Load object from local variable */
        case i_aload_0:
        case i_aload_1:
        case i_aload_2:
        case i_aload_3: {
            int32_t param = current - i_aload_0;
            object_t *obj = locals[param].entry.ptr_value;
            push_ref(op_stack, obj);
            pc += 1;
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

        /* Store long into local variable */
        case i_lstore: {
            int32_t param = code_buf[pc + 1];
            int64_t stored = pop_int(op_stack);
            locals[param].entry.long_value = stored;
            locals[param].type = STACK_ENTRY_LONG;

            pc += 2;
            break;
        }

        /* Store object from local variable */
        case i_astore: {
            int32_t param = code_buf[pc + 1];
            locals[param].entry.ptr_value = pop_ref(op_stack);
            locals[param].type = STACK_ENTRY_REF;
            pc += 2;
            break;
        }

        /* Store long into local variable */
        case i_lstore_0:
        case i_lstore_1:
        case i_lstore_2:
        case i_lstore_3: {
            int32_t param = current - i_lstore_0;
            int64_t stored = pop_int(op_stack);
            locals[param].entry.long_value = stored;
            locals[param].type = STACK_ENTRY_LONG;

            pc += 1;
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

        /* Store object from local variable */
        case i_astore_0:
        case i_astore_1:
        case i_astore_2:
        case i_astore_3: {
            int32_t param = current - i_astore_0;
            locals[param].entry.ptr_value = pop_ref(op_stack);
            locals[param].type = STACK_ENTRY_REF;
            pc += 1;
            break;
        }

        /* discard the top value on the stack */
        case i_pop: {
            op_stack->size--;
            pc += 1;
            break;
        }

        /* duplicate the value on top of the stack */
        case i_dup: {
            op_stack->store[op_stack->size] =
                op_stack->store[op_stack->size - 1];
            op_stack->size++;
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

        /* Convert int to long */
        case i_i2l: {
            int32_t stored = pop_int(op_stack);
            push_long(op_stack, (int64_t) stored);

            pc += 1;
            break;
        }

        /* Convert int to char */
        case i_l2i: {
            int64_t stored = pop_int(op_stack);
            push_int(op_stack, (int32_t) stored);

            pc += 1;
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

        /* Add long */
        case i_ladd: {
            int64_t op1 = pop_int(op_stack);
            int64_t op2 = pop_int(op_stack);

            push_long(op_stack, op1 + op2);
            pc += 1;
            break;
        }

        /* Subtract int */
        case i_isub:
            isub(op_stack);
            pc += 1;
            break;

        /* Subtract long */
        case i_lsub: {
            int64_t op1 = pop_int(op_stack);
            int64_t op2 = pop_int(op_stack);

            push_long(op_stack, op2 - op1);
            pc += 1;
            break;
        }

        /* Multiply int */
        case i_imul:
            imul(op_stack);
            pc += 1;
            break;

        /* Multiply long */
        case i_lmul: {
            int64_t op1 = pop_int(op_stack);
            int64_t op2 = pop_int(op_stack);

            push_long(op_stack, op1 * op2);
            pc += 1;
            break;
        }

        /* Divide int */
        case i_idiv:
            idiv(op_stack);
            pc += 1;
            break;

        /* Divide long */
        case i_ldiv: {
            int64_t op1 = pop_int(op_stack);
            int64_t op2 = pop_int(op_stack);

            push_long(op_stack, op2 / op1);
            pc += 1;
            break;
        }

        /* Remainder int */
        case i_irem:
            irem(op_stack);
            pc += 1;
            break;

        /* Remainder long */
        case i_lrem: {
            int64_t op1 = pop_int(op_stack);
            int64_t op2 = pop_int(op_stack);

            push_long(op_stack, op2 % op1);
            pc += 1;
            break;
        }

        /* Negate int */
        case i_ineg:
            ineg(op_stack);
            pc += 1;
            break;

        /* Get static field from class */
        case i_getstatic: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            uint16_t index = ((param1 << 8) | param2);

            char *field_name, *field_descriptor, *class_name;
            class_name = find_field_info_from_index(index, clazz, &field_name,
                                                    &field_descriptor);

            /* skip java.lang.System in order to support java print
             * method */
            if (!strcmp(class_name, "java/lang/System")) {
                pc += 3;
                break;
            }

            class_file_t *target_class =
                find_or_add_class_to_heap(class_name, prefix);
            field_t *field =
                find_field(field_name, field_descriptor, target_class);

            switch (field_descriptor[0]) {
            case 'B':
                /* signed byte */
                push_byte(op_stack, field->static_var->value.char_value);
                break;
            case 'C':
                /* FIXME: complete Unicode handling */
                /* unicode character code */
                push_short(op_stack, field->static_var->value.short_value);
                break;
            case 'I':
                /* integer */
                push_int(op_stack, field->static_var->value.int_value);
                break;
            case 'J':
                /* long integer */
                push_long(op_stack, field->static_var->value.long_value);
                break;
            case 'S':
                /* signed short */
                push_short(op_stack, field->static_var->value.short_value);
                break;
            case 'Z':
                /* true or false */
                push_byte(op_stack, field->static_var->value.char_value);
                break;
            case 'L':
                /* an instance of class */
                push_ref(op_stack, field->static_var->value.ptr_value);
                break;
            default:
                fprintf(stderr, "Unknown field descriptor %c\n",
                        field_descriptor[0]);
                exit(1);
            }
            pc += 3;
            break;
        }

        /* Put static field to class */
        case i_putstatic: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            uint16_t index = ((param1 << 8) | param2);

            char *field_name, *field_descriptor, *class_name;
            class_name = find_field_info_from_index(index, clazz, &field_name,
                                                    &field_descriptor);

            class_file_t *target_class =
                find_or_add_class_to_heap(class_name, prefix);
            field_t *field =
                find_field(field_name, field_descriptor, target_class);

            switch (field_descriptor[0]) {
            case 'B':
                /* signed byte */
                field->static_var->value.char_value = (u1) pop_int(op_stack);
                field->static_var->type = VAR_BYTE;
                break;
            case 'C':
                /* FIXME: complete Unicode handling */
                /* unicode character code */
                field->static_var->value.char_value = (u2) pop_int(op_stack);
                field->static_var->type = VAR_SHORT;
                break;
            case 'I':
                /* integer */
                field->static_var->value.int_value = (u4) pop_int(op_stack);
                field->static_var->type = VAR_INT;
                break;
            case 'J':
                /* long integer */
                field->static_var->value.long_value = (u8) pop_int(op_stack);
                field->static_var->type = VAR_LONG;
                break;
            case 'S':
                /* signed short */
                field->static_var->value.short_value = (u2) pop_int(op_stack);
                field->static_var->type = VAR_SHORT;
                break;
            case 'Z':
                /* true or false */
                field->static_var->value.char_value = (u1) pop_int(op_stack);
                field->static_var->type = VAR_BYTE;
                break;
            case 'L':
                /* an instance of class ClassName */
                field->static_var->value.ptr_value = pop_ref(op_stack);
                field->static_var->type = VAR_PTR;
                break;
            default:
                fprintf(stderr, "Unknown field descriptor %c\n",
                        field_descriptor[0]);
                exit(1);
            }
            pc += 3;
            break;
        }

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

        /* Push long constant */
        case i_lconst_0:
        case i_lconst_1: {
            push_long(op_stack, current - i_lconst_0);
            pc += 1;
            break;
        }

        /* Push short */
        case i_sipush:
            sipush(op_stack, pc, code_buf);
            pc += 3;
            break;

        /* Fetch field from object */
        case i_getfield: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            uint16_t index = ((param1 << 8) | param2);

            object_t *obj = pop_ref(op_stack);
            char *field_name, *field_descriptor;
            find_field_info_from_index(index, clazz, &field_name,
                                       &field_descriptor);

            variable_t *addr = find_field_addr(obj, field_name);

            switch (field_descriptor[0]) {
            case 'I':
                push_int(op_stack, addr->value.int_value);
                break;
            case 'J':
                push_long(op_stack, addr->value.long_value);
                break;
            case 'L':
                push_ref(op_stack, addr->value.ptr_value);
                break;
            default:
                assert(0 && "Only support integer and reference field");
                break;
            }
            pc += 3;
            break;
        }

        /* Set field in object */
        case i_putfield: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            uint16_t index = ((param1 << 8) | param2);
            int64_t value = 0;
            void *addr = NULL;

            /* get prepared value from the stack */
            stack_entry_t element = top(op_stack);
            switch (element.type) {
            /* integer */
            case STACK_ENTRY_INT:
            case STACK_ENTRY_SHORT:
            case STACK_ENTRY_BYTE:
            case STACK_ENTRY_LONG:
                value = pop_int(op_stack);
                break;
            case STACK_ENTRY_REF:
                addr = pop_ref(op_stack);
                break;
            default:
                assert(0 && "Only support integer and reference field");
                break;
            }

            object_t *obj = pop_ref(op_stack);

            /* update value into object's field */
            char *field_name, *field_descriptor;
            find_field_info_from_index(index, clazz, &field_name,
                                       &field_descriptor);

            variable_t *var = find_field_addr(obj, field_name);

            switch (field_descriptor[0]) {
            case 'I':
                var->value.int_value = (int32_t) value;
                var->type = VAR_INT;
                break;
            case 'J':
                var->value.long_value = value;
                var->type = VAR_LONG;
                break;
            case 'L':
                var->value.ptr_value = addr;
                var->type = VAR_PTR;
                break;
            default:
                assert(0 && "Only support integer and reference field");
                break;
            }
            pc += 3;
            break;
        }

        /* create new object */
        case i_new: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            uint16_t index = ((param1 << 8) | param2);

            char *class_name = find_class_name_from_index(index, clazz);
            class_file_t *target_class =
                find_or_add_class_to_heap(class_name, prefix);
            assert(target_class && "Failed to load class in new");

            object_t *object = create_object(target_class);
            push_ref(op_stack, object);

            pc += 3;
            break;
        }

        /* Invoke object constructor method */
        case i_invokespecial: {
            uint8_t param1 = code_buf[pc + 1], param2 = code_buf[pc + 2];
            uint16_t index = ((param1 << 8) | param2);

            /* the method to be called */
            char *method_name, *method_descriptor, *class_name;
            class_name = find_method_info_from_index(index, clazz, &method_name,
                                                     &method_descriptor);

            /* java.lang.Object is the parent for every object, so every object
             * will finally call java.lang.Object's constructor */
            if (!strcmp(class_name, "java/lang/Object")) {
                pop_ref(op_stack);
                pc += 3;
                break;
            }

            class_file_t *target_class =
                find_or_add_class_to_heap(class_name, prefix);
            assert(target_class && "Failed to load class in i_invokespecial");

            /* find constructor method from class */
            method_t *constructor =
                find_method(method_name, method_descriptor, target_class);

            /* prepare local variables */
            uint16_t num_params = get_number_of_parameters(constructor);
            local_variable_t own_locals[constructor->code.max_locals];
            for (int i = num_params; i >= 1; i--) {
                pop_to_local(op_stack, &own_locals[i]);
            }

            /* first argument must be object itself */
            object_t *obj = pop_ref(op_stack);
            own_locals[0].entry.ptr_value = obj;
            own_locals[0].type = STACK_ENTRY_REF;

            stack_entry_t *exec_res =
                execute(constructor, own_locals, target_class);
            assert(exec_res->type == STACK_ENTRY_NONE &&
                   "A constructor must not return a value.");
            free(exec_res);

            pc += 3;
            break;
        }

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
    class_file_t *clazz = malloc(sizeof(class_file_t));
    *clazz = get_class(class_file);

    int error = fclose(class_file);
    assert(!error && "Failed to close file");

    init_class_heap();
    init_object_heap();

    add_class(clazz, argv[1]);
    char *match = strrchr(argv[1], '/');
    if (match != NULL) {
        /* get the prefix from path */
        prefix = malloc((match - argv[1] + 2));
        strncpy(prefix, argv[1], match - argv[1] + 1);
        prefix[match - argv[1] + 1] = '\0';
    }

    /* execute the main method if found */
    method_t *main_method =
        find_method("main", "([Ljava/lang/String;)V", clazz);
    assert(main_method && "Missing main() method");

    /* FIXME: locals[0] contains a reference to String[] args, but right now
     * we lack of the support for java.lang.Object. Leave it uninitialized.
     */
    local_variable_t locals[main_method->code.max_locals];
    stack_entry_t *result = execute(main_method, locals, clazz);
    assert(result->type == STACK_ENTRY_NONE && "main() should return void");
    free(result);

    free_object_heap();
    free_class_heap();
    free(prefix);

    return 0;
}
