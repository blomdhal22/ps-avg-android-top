#ifndef _LIST_H
#define _LIST_H

#include <stddef.h>

struct listnode {
    struct listnode* prev;
    struct listnode* next;
};

#define node_to_item(node, container, member)   \
    (container *)(((char*)(node))- offsetof(container, member))

#define list_declare(name) \
    struct listnode name = {    \
        .prev = &name,          \
        .next = &name,          \
    }

#define list_for_each(node, head) \
    for (node = (head)->next; node != head; node = (node)->next)

#define list_for_reverse(node, head) \
    for (node = (head)->prev; node != head; node = (node)->prev)

#define list_for_each_safe(node, n, head) \
    for (node = (head)->next, n = (node)->next; node != head; node = n, n = (node)->next)

static inline void list_init(struct listnode* node)
{
    node->prev = node;
    node->next = node;
}

static inline void list_add_head(struct listnode* head, struct listnode* item)
{
    item->prev = head;
    item->next = head->next;
    head->next->prev = item;
    head->next = item;
}

static inline void list_add_tail(struct listnode* head, struct listnode* item)
{
    item->next = head;
    item->prev = head->prev;
    head->prev->next = item;
    head->prev = item;
}

static inline void list_del(struct listnode* item)
{
    item->prev = item->next;
    item->next->prev = item->prev;
}

#define list_empty(list)    ((list) == (list)->next)
#define list_head(list)     ((list)->next)
#define list_tail(list)     ((list)->prev)

#endif