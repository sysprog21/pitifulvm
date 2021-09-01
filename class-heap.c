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

bool find_or_add_class_to_heap(char *class_name,
                               char *prefix,
                               class_file_t **target_class)
{
    bool added = false;
    *target_class = find_class_from_heap(class_name);

    /* if class is not found, adding specific class path and finding
     * again. This can be removed by recording path infomation in class
     * heap
     */
    if (!*target_class && prefix) {
        char *tmp = malloc(strlen(class_name) + strlen(prefix) + 1);
        strcpy(tmp, prefix);
        strcat(tmp, class_name);
        *target_class = find_class_from_heap(tmp);

        free(tmp);
    }

    /* if class is still not found, meaning that this class is not
     * loaded in class heap, so add it to the class heap. */
    if (!*target_class) {
        char *tmp;
        if (prefix) {
            tmp = malloc(
                (strlen(class_name) + strlen(".class") + strlen(prefix) + 1));
            strcpy(tmp, prefix);
            strcat(tmp, class_name);
        } else {
            tmp = malloc(strlen(class_name) + strlen(".class") + 1);
            strcpy(tmp, class_name);
        }
        /* attempt to read given class file */
        FILE *class_file = fopen(strcat(tmp, ".class"), "r");
        assert(class_file && "Failed to open file");

        /* parse the class file */
        *target_class = malloc(sizeof(class_file_t));
        **target_class = get_class(class_file);
        int error = fclose(class_file);
        assert(!error && "Failed to close file");
        add_class(*target_class, tmp);
        free(tmp);
        added = true;
    }

    return added;
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
        free(class_heap.class_info[i]->clazz->info);

        field_t *field = class_heap.class_info[i]->clazz->fields;
        for (u2 j = 0; j < class_heap.class_info[i]->clazz->fields_count;
             j++, field++)
            free(field->static_var);
        free(class_heap.class_info[i]->clazz->fields);

        for (method_t *method = class_heap.class_info[i]->clazz->methods;
             method->name; method++) {
            free(method->code.code);
        }
        free(class_heap.class_info[i]->clazz->methods);

        bootmethods_attr_t *bootstrap =
            class_heap.class_info[i]->clazz->bootstrap;
        if (bootstrap) {
            for (u2 j = 0; j < bootstrap->num_bootstrap_methods; j++)
                free(bootstrap->bootstrap_methods[j].bootstrap_arguments);
            free(bootstrap->bootstrap_methods);
            free(bootstrap);
        }

        free(class_heap.class_info[i]->clazz);
        free(class_heap.class_info[i]->name);
        free(class_heap.class_info[i]);
    }
    free(class_heap.class_info);
}
