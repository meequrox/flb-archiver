#ifndef FLB_ARCHIVER_LINKED_LIST_H
#define FLB_ARCHIVER_LINKED_LIST_H

#include <stddef.h>

typedef struct flb_list_node_t {
    char* value;
    size_t value_len;
    struct flb_list_node_t* next;
} flb_list_node;

flb_list_node* flb_list_create_node(const char* value);
void flb_list_free(flb_list_node* list);

flb_list_node* flb_list_insert_front(flb_list_node* list, const char* value);
__attribute__((unused)) flb_list_node* flb_list_lookup(flb_list_node* list, const char* value);
__attribute__((unused)) flb_list_node* flb_list_delete(flb_list_node* list, const char* value);

#endif  // FLB_ARCHIVER_LINKED_LIST_H
