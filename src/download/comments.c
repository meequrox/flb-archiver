#include "download/comments.h"

#include <string.h>

#include "data_structures/linked_list.h"
#include "logger/logger.h"

static char* get_array_start(xmlNode* node) {
    const char* content = (char*) xmlNodeGetContent(node);

    // Example: ...постов\nlet posts = ["1","109","111","121","132","4375"];\n...
    //                     ^            ^
    //                start_ptr(1)      |
    //                             start_ptr(2)

    const char needle[] = "let posts = [";
    const size_t needle_len = sizeof(needle) / sizeof(*needle) - 1;

    char* start_ptr = strstr(content, needle);
    if (!start_ptr) {
        return NULL;
    }

    start_ptr += needle_len;
    return start_ptr;
}

static flb_list_node* parse_array(const char* ptr) {
    char* str = strdup(ptr);
    if (!str) {
        FLB_LOG_ERROR("Can't create string copy");
        return NULL;
    }

    strchr(str, ']')[0] = '\0';

    flb_list_node* list = NULL;

    const char delimiter[] = ",";
    char* token = strtok(str, delimiter);

    while (token != NULL) {
        if (token[0] == '"') {
            ++token;
        }

        const size_t token_len = strlen(token);

        if (token[token_len - 1] == '"') {
            token[token_len - 1] = '\0';
        }

        list = flb_list_insert_front(list, token);
        token = strtok(NULL, delimiter);
    }

    free(str);
    return list;
}

static flb_list_node* get_comments_ids(xmlXPathContext* context) {
    const xmlChar* expr = (xmlChar*) "//script[contains(., '// Подгрузка комментариев')]";
    xmlXPathObject* result = xmlXPathEvalExpression(expr, context);
    xmlNodeSet* nodes = result->nodesetval;

    if (nodes->nodeNr != 1) {
        // nodeNr = 0 => not found
        // nodeNr > 1 is not possible
        return NULL;
    }

    xmlNode* node = nodes->nodeTab[0];
    FLB_LOG_INFO("Found tag '%s' with comments IDs", (char*) node->name);

    char* array_start_ptr = get_array_start(node);
    flb_list_node* list = parse_array(array_start_ptr);

    xmlXPathFreeObject(result);
    return list;
}

void TMP_flb_list_print(flb_list_node* list) {
    while (list) {
        printf("%s -> ", list->value);
        list = list->next;
    }

    printf("NULL\n");
}

int fetch_comments(CURL* curl_handle, xmlXPathContext* context) {
    flb_list_node* ids_list = get_comments_ids(context);

    if (!ids_list) {
        FLB_LOG_ERROR("Can't get comments IDs");
        return 1;
    }

    // TODO(1): fetch comments HTML
    // TODO(2): remove TMP_flb_list_print() function
    // TODO(3): insert comments HTML into context doc
    TMP_flb_list_print(ids_list);
    (void) curl_handle;

    // TODO(4): return 0
    flb_list_free(ids_list);
    return 1;
}
