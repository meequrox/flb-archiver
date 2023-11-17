#include "data_structures/linked_list.h"

#include <stdlib.h>
#include <string.h>

flb_list_node* flb_list_create_node(const char* value) {
    if (!value) {
        return NULL;
    }

    flb_list_node* node = malloc(sizeof(flb_list_node));

    if (!node) {
        return NULL;
    }

    const size_t value_len = strlen(value);
    char* value_copy = strndup(value, value_len);

    if (!value_copy) {
        free(node);
        return NULL;
    }

    node->value = value_copy;
    node->value_len = value_len;
    node->next = NULL;

    return node;
}

flb_list_node* flb_list_insert_front(flb_list_node* list, const char* value) {
    flb_list_node* new_node = flb_list_create_node(value);

    if (!new_node) {
        return list;
    }

    new_node->next = list;
    return new_node;
}

__attribute__((unused)) flb_list_node* flb_list_lookup(flb_list_node* list, const char* value) {
    while (list) {
        if (strcmp(list->value, value) == 0) {
            return list;
        }

        list = list->next;
    }

    return NULL;
}

__attribute__((unused)) flb_list_node* flb_list_delete(flb_list_node* list, const char* value) {
    for (flb_list_node *prev = NULL, *cur = list; cur; cur = cur->next) {
        if (strcmp(cur->value, value) == 0) {
            if (prev == NULL) {
                list = cur->next;
            } else {
                prev->next = cur->next;
            }

            free(cur->value);
            cur->value = NULL;
            cur->value_len = 0;
            free(cur);

            return list;
        }

        prev = cur;
    }

    return NULL;
}

void flb_list_free(flb_list_node* list) {
    flb_list_node* prev = NULL;

    while (list) {
        prev = list;
        list = list->next;

        free(prev->value);
        prev->value = NULL;
        prev->value_len = 0;
        free(prev);
    }
}
