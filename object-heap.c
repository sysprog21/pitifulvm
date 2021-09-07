#include "object-heap.h"

/* FIXME: use dynamic structure to grow heap size dynamically */
#define MAX_HEAP_SIZE 5000

static object_heap_t object_heap;

void init_object_heap()
{
    /* max contain 5000 objects */
    object_heap.objects = malloc(sizeof(object_t *) * MAX_HEAP_SIZE);
    object_heap.length = 0;
}

/**
 * Create an java object.
 *
 * @param clazz the list of classes that contains the created class and all its
 * parent classess
 * @return the object that wanted to be created
 */
object_t *create_object(class_file_t *clazz)
{
    object_t *new_obj = NULL, *parent = NULL;
    class_file_t *pos;
    list_for_each (pos, clazz) {
        new_obj = malloc(sizeof(object_t));
        new_obj->fields_count = pos->fields_count;
        new_obj->value = calloc(sizeof(variable_t), new_obj->fields_count);
        new_obj->parent = parent;
        for (int i = 0; i < pos->fields_count; ++i) {
            new_obj->value[i].type = VAR_NONE;
        }
        new_obj->class = pos;
        parent = new_obj;
    }
    /* only store object that really is needed in object heap */
    object_heap.objects[object_heap.length++] = new_obj;

    return new_obj;
}

char *create_string(class_file_t *clazz, char *src)
{
    size_t len = strlen(src);
    char *dest = calloc((len + 1), sizeof(char));
    strncpy(dest, src, len);

    object_t *str_obj = malloc(sizeof(object_t));
    str_obj->value = malloc(sizeof(variable_t));
    str_obj->value->type = VAR_STR_PTR;
    str_obj->value->value.ptr_value = dest;
    str_obj->class = clazz;
    str_obj->parent = NULL;
    str_obj->fields_count = 1;

    object_heap.objects[object_heap.length++] = str_obj;

    return dest;
}

/**
 * Build an array recursively.
 *
 * @param depth the current dfs depth
 * @param dimension number of dimension in the array
 * @param n_elements the array represents number of element in each dimension
 * @param type_size element size of the array
 * @return the array that wanted be created
 */
void **build_array(uint8_t depth,
                   int dimension,
                   int *n_elements,
                   size_t type_size)
{
    void **arr;
    if (depth == dimension - 1) {
        arr = calloc(n_elements[depth], type_size);
    } else {
        arr = calloc(n_elements[depth], sizeof(void *));
        for (int i = 0; i < n_elements[depth]; ++i) {
            *(arr + i) =
                build_array(depth + 1, dimension, n_elements, type_size);
        }
    }
    return arr;
}

/**
 * Free the array recursively.
 *
 * @param obj the object that contains the array
 * @param depth the current dfs depth
 * @param dimension number of dimension in the array
 * @return arr the array that will be freed
 */
void free_array(object_t *obj, uint8_t depth, int dimension, void **arr)
{
    if (depth != dimension - 1) {
        for (int i = 0; i < ((int *) (obj->value[1].value.ptr_value))[depth];
             ++i) {
            free_array(obj, depth + 1, dimension, *(arr + i));
        }
    }
    free(arr);
}

/**
 * Create an array object.
 *
 * @param clazz class of the element in the array, but only meaningful when
 * types are not primitive types
 * @param dimension number of dimension in the array
 * @param n_elements the array represents number of element in each dimension
 * @param type_size element size of the array
 * @return the array that wanted be created
 */
void *create_array(class_file_t *clazz,
                   uint8_t dimension,
                   int *n_elements,
                   size_t type_size)
{
    void *arr = build_array(0, dimension, n_elements, type_size);
    object_t *arr_obj = malloc(sizeof(object_t));
    arr_obj->value = malloc(sizeof(variable_t) * 3);
    /* element 0: array memory
     * element 1: number of elements in each dimension
     * element 2: total dimensions in the array
     */
    arr_obj->value[0].type = VAR_ARRAY_PTR;
    arr_obj->value[0].value.ptr_value = arr;
    arr_obj->value[1].type = VAR_PTR;
    arr_obj->value[1].value.ptr_value = n_elements;
    arr_obj->value[2].type = VAR_BYTE;
    arr_obj->value[2].value.char_value = dimension;
    arr_obj->class = clazz;
    arr_obj->parent = NULL;
    arr_obj->fields_count = 1;

    object_heap.objects[object_heap.length++] = arr_obj;

    return (void *) arr;
}

variable_t *find_field_addr(object_t *obj, char *name)
{
    field_t *field = obj->class->fields;
    for (u2 i = 0; i < obj->class->fields_count; i++, field++) {
        if (!strcmp(field->name, name)) {
            return &obj->value[i];
        }
    }
    return NULL;
}

void free_object_heap()
{
    for (int i = 0; i < object_heap.length; ++i) {
        /* free object and all its parent */
        for (object_t *cur = object_heap.objects[i], *next; cur; cur = next) {
            next = cur->parent;
            if (cur->value) {
                if (cur->value->type == VAR_STR_PTR) {
                    free(cur->value->value.ptr_value);
                } else if (cur->value->type == VAR_ARRAY_PTR) {
                    free_array(cur, 0, cur->value[2].value.char_value,
                               cur->value[0].value.ptr_value);
                    free(cur->value[1].value.ptr_value);
                }
            }
            free(cur->value);
            free(cur);
        }
    }
    free(object_heap.objects);
}