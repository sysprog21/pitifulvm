#include "class-heap.h"

/* FIXME: use dynamic structure to grow heap size dynamically */
#define MAX_HEAP_SIZE 100

static class_heap_t class_heap;

void init_class_heap()
{
    /* max contain 100 class file */
    class_heap.class_info = malloc(sizeof(meta_class_t *) * MAX_HEAP_SIZE);
    class_heap.length = 0;
}

/* the name parameter should contain ".class" suffix and be cut in this function
 */
void add_class(class_file_t *clazz, char *name)
{
    meta_class_t *meta_class = malloc(sizeof(meta_class_t));
    meta_class->clazz = clazz;
    /* assume class file is in the same directory */
    /* -6 mean clip ".class" in posfix */
    meta_class->name = malloc((strlen(name) + 1 - 6));
    strncpy(meta_class->name, name, strlen(name) - 6);
    meta_class->name[strlen(name) - 6] = '\0';
    class_heap.class_info[class_heap.length++] = meta_class;
}

class_file_t *find_class_from_heap(char *value)
{
    for (int i = 0; i < class_heap.length; ++i) {
        if (!strcmp(class_heap.class_info[i]->name, value))
            return class_heap.class_info[i]->clazz;
    }
    return NULL;
}

void free_class_heap()
{
    for (int i = 0; i < class_heap.length; ++i) {
        const_pool_info *constant =
            class_heap.class_info[i]->clazz->constant_pool.constant_pool;
        for (u2 j = 0; j < class_heap.class_info[i]->clazz->constant_pool.count;
             j++, constant++) {
            free(constant->info);
        }
        free(class_heap.class_info[i]->clazz->constant_pool.constant_pool);
        free(class_heap.class_info[i]->clazz->fields);

        for (method_t *method = class_heap.class_info[i]->clazz->methods;
             method->name; method++) {
            free(method->code.code);
        }
        free(class_heap.class_info[i]->clazz->methods);

        free(class_heap.class_info[i]->clazz);
        free(class_heap.class_info[i]->name);
        free(class_heap.class_info[i]);
    }
    free(class_heap.class_info);
}
