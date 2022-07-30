#include "stack.h"

void init_stack(stack_frame_t *stack, size_t entry_size)
{
    memset(stack, 0, sizeof(stack_frame_t));
    stack->max_size = entry_size;
    stack->store = calloc(sizeof(stack_entry_t), entry_size);
    stack->size = 0;
}

void push_byte(stack_frame_t *stack, int8_t value)
{
    stack->store[stack->size].entry.char_value = value;
    stack->store[stack->size].type = STACK_ENTRY_BYTE;
    stack->size++;
}

void push_short(stack_frame_t *stack, int16_t value)
{
    stack->store[stack->size].entry.short_value = value;
    stack->store[stack->size].type = STACK_ENTRY_SHORT;
    stack->size++;
}

void push_int(stack_frame_t *stack, int32_t value)
{
    stack->store[stack->size].entry.int_value = value;
    stack->store[stack->size].type = STACK_ENTRY_INT;
    stack->size++;
}

void push_long(stack_frame_t *stack, int64_t value)
{
    stack->store[stack->size].entry.long_value = value;
    stack->store[stack->size].type = STACK_ENTRY_LONG;
    stack->size++;
}

void push_ref(stack_frame_t *stack, void *addr)
{
    stack->store[stack->size].entry.ptr_value = addr;
    stack->store[stack->size].type = STACK_ENTRY_REF;
    stack->size++;
}

stack_entry_t top(stack_frame_t *stack)
{
    return stack->store[stack->size - 1];
}

/* pop top of stack value and convert to 64 bits integer */
int64_t stack_to_int(value_t *entry, size_t size)
{
    switch (size) {
    /* int8_t */
    case 1:
        return (int8_t) entry->long_value;
    /* int16_t */
    case 2:
        return (int16_t) entry->long_value;
    /* int32_t */
    case 4:
        return (int32_t) entry->long_value;
    /* int64_t */
    case 8:
        return (int64_t) entry->long_value;
    default:
        assert("stack entry not an interger");
        return -1;
    }
}

int64_t pop_int(stack_frame_t *stack)
{
    size_t size = get_type_size(stack->store[stack->size - 1].type);
    int64_t value = stack_to_int(&stack->store[stack->size - 1].entry, size);
    stack->size--;
    return value;
}

void *pop_ref(stack_frame_t *stack)
{
    return stack->store[--stack->size].entry.ptr_value;
}

void pop_to_local(stack_frame_t *stack, local_variable_t *locals)
{
    stack_entry_type_t type = stack->store[stack->size - 1].type;
    /* push value from stack to locals */
    if (type >= STACK_ENTRY_BYTE && type <= STACK_ENTRY_LONG) {
        locals->entry.long_value = pop_int(stack);
        locals->type = STACK_ENTRY_LONG;
    } else if (type == STACK_ENTRY_REF) {
        locals->entry.ptr_value = pop_ref(stack);
        locals->type = STACK_ENTRY_REF;
    }
}

size_t get_type_size(stack_entry_type_t type)
{
    size_t size = 0;
    switch (type) {
    case STACK_ENTRY_NONE:
        /* pass */
        break;
    case STACK_ENTRY_BYTE:
        size = sizeof(int8_t);
        break;
    case STACK_ENTRY_SHORT:
        size = sizeof(int16_t);
        break;
    case STACK_ENTRY_INT:
        size = sizeof(int32_t);
        break;
    case STACK_ENTRY_REF:
        size = sizeof(size_t);
        break;
    case STACK_ENTRY_LONG:
        size = sizeof(int64_t);
        break;
    case STACK_ENTRY_DOUBLE:
        size = sizeof(double);
        break;
    case STACK_ENTRY_FLOAT:
        size = sizeof(float);
        break;
    default:
        assert("unable to recognize stack entry's tag");
    }
    return size;
}
