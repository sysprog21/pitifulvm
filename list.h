#pragma once

#define init_list(list) list->prev = list->next = list;

#define list_is_head(list, head) (list == head)

#define list_add(new, head) \
    head->next->prev = new; \
    new->next = head->next; \
    new->prev = head;       \
    head->next = new;

#define list_del(entry)              \
    entry->prev->next = entry->next; \
    entry->next->prev = entry->prev;

#define list_for_each(pos, head) \
    for (pos = (head)->next; !list_is_head(pos, (head)); pos = pos->next)
