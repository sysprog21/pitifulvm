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
            free(cur->value);
            free(cur);
        }
    }
    free(object_heap.objects);
}