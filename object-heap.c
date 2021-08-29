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

object_t *create_object(class_file_t *clazz)
{
    object_t *new_obj = malloc(sizeof(object_t));
    new_obj->fields_count = clazz->fields_count;
    new_obj->value = calloc(sizeof(variable_t), new_obj->fields_count);
    for (int i = 0; i < clazz->fields_count; ++i) {
        new_obj->value[i].type = VAR_NONE;
    }
    new_obj->class = clazz;
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
    assert(0 && "Can't find field in the object");
    return NULL;
}

void free_object_heap()
{
    for (int i = 0; i < object_heap.length; ++i) {
        free(object_heap.objects[i]->value);
        free(object_heap.objects[i]);
    }
    free(object_heap.objects);
}