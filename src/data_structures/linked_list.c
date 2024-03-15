#include "data_structures/linked_list.h"

#include <stdlib.h>
#include <string.h>

flb_list_node* flb_list_create_node(const char* key, const char* value) {
    if (!key) {
        return NULL;
    }

    flb_list_node* node = malloc(sizeof(flb_list_node));

    if (!node) {
        return NULL;
    }

    const size_t key_len = strlen(key);

#if defined(_WIN32)
    char* key_copy = _strdup(key);
#else
    char* key_copy = strndup(key, key_len);
#endif

    if (!key_copy) {
        free(node);
        return NULL;
    }

    char* value_copy = NULL;
    size_t value_len = 0;

    if (value) {
        value_len = strlen(value);
        value_copy = strdup(value);

        if (!value_copy) {
            free(node);
            free(key_copy);
            return NULL;
        }
    }

    node->key = key_copy;
    node->key_len = key_len;

    node->value = value_copy;
    node->value_len = value_len;

    node->next = NULL;

    return node;
}

flb_list_node* flb_list_insert_front(flb_list_node* list, const char* key, const char* value) {
    flb_list_node* new_node = flb_list_create_node(key, value);

    if (!new_node) {
        return list;
    }

    new_node->next = list;
    return new_node;
}

__attribute__((unused)) flb_list_node* flb_list_lookup(flb_list_node* list, const char* key) {
    while (list) {
        if (strcmp(list->key, key) == 0) {
            return list;
        }

        list = list->next;
    }

    return NULL;
}

__attribute__((unused)) flb_list_node* flb_list_delete(flb_list_node* list, const char* key) {
    for (flb_list_node *prev = NULL, *cur = list; cur; cur = cur->next) {
        if (strcmp(cur->key, key) == 0) {
            if (prev == NULL) {
                list = cur->next;
            } else {
                prev->next = cur->next;
            }

            free(cur->key);
            cur->key = NULL;
            cur->key_len = 0;
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

        free(prev->key);
        prev->key = NULL;
        prev->key_len = 0;

        free(prev->value);
        prev->value = NULL;

        free(prev);
    }
}

int flb_list_foreach3(flb_list_node* list, int (*fun)(void*, const char*, const char*),
                      void* first_arg) {
    if (!list || !fun) {
        return 1;
    }

    int rc = 0;
    flb_list_node* prev = NULL;

    while (list) {
        prev = list;
        list = list->next;

        rc += fun(first_arg, prev->key, prev->value);
    }

    return rc;
}
