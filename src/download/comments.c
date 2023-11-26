#include "download/comments.h"

#include <libxml/HTMLparser.h>
#include <libxml/xpathInternals.h>
#include <stdlib.h>
#include <string.h>

#include "data_structures/linked_list.h"
#include "logger/logger.h"
#include "memory/memory.h"

static char* get_array_start(xmlNode* node) {
    // Expected != NULL: node

    // Example: ...постов\nlet posts = ["1","109","111","121","132","4375"];\n...
    //                     ^            ^
    //                start_ptr(1)      |
    //                             start_ptr(2)

    const char needle[] = "let posts = [";
    const size_t needle_len = sizeof(needle) / sizeof(*needle) - 1;

    const char* content = (char*) xmlNodeGetContent(node);
    char* start_ptr = strstr(content, needle);

    if (start_ptr) {
        return start_ptr + needle_len;
    }

    return NULL;
}

static flb_list_node* parse_array(const char* ptr, size_t* counter) {
    // Expected != NULL: ptr, counter

    char* str = strdup(ptr);
    if (!str) {
        return NULL;
    }

    strchr(str, ']')[0] = '\0';

    flb_list_node* list = NULL;

    const char delimiter[] = ",";

    char* save_ptr = NULL;
    char* token = strtok_r(str, delimiter, &save_ptr);

    while (token != NULL) {
        if (token[0] == '"') {
            ++token;
        }

        const size_t token_len = strlen(token);

        if (token[token_len - 1] == '"') {
            token[token_len - 1] = '\0';
        }

        list = flb_list_insert_front(list, token, NULL);
        ++(*counter);

        token = strtok_r(NULL, delimiter, &save_ptr);
    }

    free(str);
    return list;
}

static flb_list_node* get_comments_ids(xmlXPathContext* context, size_t* counter) {
    // Expected != NULL: context, counter

    const xmlChar* expr = (xmlChar*) "(//script[contains(., 'let posts = [')])[1]";
    xmlXPathObject* result = xmlXPathEvalExpression(expr, context);
    xmlNodeSet* nodes = result->nodesetval;

    flb_list_node* list = NULL;

    if (nodes->nodeNr > 0) {
        xmlNode* node = nodes->nodeTab[0];
        char* array_start_ptr = get_array_start(node);

        if (array_start_ptr) {
            FLB_LOG_INFO("Found array with comments IDs");
            list = parse_array(array_start_ptr, counter);
        }
    }

    xmlXPathFreeObject(result);
    return list;
}

void strncpy_no_trunc(char* dest, const char* src, size_t n) {
    // Expected != NULL: dest

    if (src && n) {
        size_t i = 0;

        while (i < n && src[i] != '\0') {
            dest[i] = src[i];
            ++i;
        }

        while (i < n) {
            dest[i] = '\0';
            ++i;
        }
    }
}

static int concat_fields(flb_list_node* list, char* dest, const size_t dest_size, const char* field_name,
                         const size_t field_name_len) {
    // Expected != NULL: dest, field_name
    // Expected != 0: dest_size, field_name_len

    if (dest_size < field_name_len + 2) {
        FLB_LOG_ERROR("Destination buffer size is too small");
        dest[dest_size - 1] = '\0';
        return dest_size - 1;
    }

    int end_idx = dest_size - 2;
    dest[end_idx + 1] = '\0';

    while (list && end_idx > (int) field_name_len) {
        const int value_offset = list->key_len - 1;
        strncpy_no_trunc(&dest[end_idx - value_offset], list->key, list->key_len);

        const int field_name_offset = value_offset + field_name_len;
        strncpy_no_trunc(&dest[end_idx - field_name_offset], field_name, field_name_len);

        end_idx = end_idx - field_name_offset - 1;
        dest[end_idx] = '&';
        --end_idx;

        list = list->next;
    }

    dest[end_idx + 1] = '_';
    return end_idx + 2;
}

static void append_nodes_to_div(xmlNode* div_node, xmlNodeSet* nodes) {
    // Expected != NULL: div_node, nodes

    xmlNode* last_child = div_node->last;

    for (int i = 0; i < nodes->nodeNr; i++) {
        xmlNode* node = nodes->nodeTab[i];

        xmlUnlinkNode(node);
        xmlAddNextSibling(last_child, node);
        last_child = node;
    }
}

static void insert_comments(char* html, xmlXPathContext* thread_context) {
    // Expected != NULL: html, context

    const int parse_options = HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING;  // NOLINT
    xmlDoc* comments_doc = htmlReadDoc((xmlChar*) html, NULL, "UTF-8", parse_options);
    xmlXPathContext* comments_context = xmlXPathNewContext(comments_doc);

    if (!comments_doc || !comments_context) {
        FLB_LOG_ERROR("Can't read HTML comments data or create XPath context");
        xmlFreeDoc(comments_doc);
        xmlXPathFreeContext(comments_context);
        return;
    }

    xmlXPathRegisterNs(comments_context, (xmlChar*) "html", (xmlChar*) "http://www.w3.org/1999/xhtml");

    xmlXPathObject* comments_result =
        xmlXPathEvalExpression((xmlChar*) "//div[@class='commentTop']", comments_context);
    xmlXPathObject* thread_result =
        xmlXPathEvalExpression((xmlChar*) "(//div[@class='comments'])[1]", thread_context);

    if (comments_result && thread_result) {
        xmlNodeSet* comments_nodes = comments_result->nodesetval;
        xmlNodeSet* thread_nodes = thread_result->nodesetval;

        if (thread_nodes->nodeNr > 0) {
            append_nodes_to_div(thread_nodes->nodeTab[0], comments_nodes);

            FLB_LOG_INFO("Inserted %d comment nodes", comments_nodes->nodeNr);
        }
    } else {
        FLB_LOG_ERROR("Can't eval XPath to parse comments HTML");
    }

    xmlFreeDoc(comments_doc);
    xmlXPathFreeContext(comments_context);
    xmlXPathFreeObject(comments_result);
    xmlXPathFreeObject(thread_result);
}

void include_comments_setup(CURL* curl_handle, const char* url, void* memory, const char* post_fields) {
    // Expected != NULL: curl_handle, url, memory, post_fields

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, flb_write_memory_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, memory);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, post_fields);
}

int include_comments(CURL* curl_handle, xmlXPathContext* context) {
    // Expected != NULL: curl_handle, context

    size_t counter = 0;
    flb_list_node* ids_list = get_comments_ids(context, &counter);

    if (!ids_list && counter == 0) {
        return 0;
    }

    const char field_name[] = "currentPosts[]=";
    const size_t field_name_len = sizeof(field_name) / sizeof(*field_name) - 1;

    const size_t uint_max_digits = 10;
    const size_t post_fields_size = counter * (field_name_len + uint_max_digits + 1) + 1;

    // "currentPosts[]=6001&currentPosts[]=6002"
    char* post_fields = malloc(post_fields_size * sizeof(*post_fields));
    if (!post_fields) {
        FLB_LOG_ERROR("Can't allocate POSTFIELDS data");
        return 1;
    }

    const int post_fields_offset =
        concat_fields(ids_list, post_fields, post_fields_size, field_name, field_name_len);
    char* post_fields_ptr = post_fields + post_fields_offset;

    flb_list_free(ids_list);
    ids_list = NULL;

    const size_t initial_data_size = 16 * 1024 + 1;
    flb_memstruct_t memory = {(char*) malloc(initial_data_size), 0, initial_data_size};

    extern const char kBaseUrl[];
    extern const size_t kBaseUrlLen;

    const char script_name[] = "selectPostComments.php";

    const size_t url_size = kBaseUrlLen + sizeof(script_name) / sizeof(*script_name);
    char url[url_size];
    snprintf(url, url_size, "%s%s", kBaseUrl, script_name);

    include_comments_setup(curl_handle, url, &memory, post_fields_ptr);
    CURLcode response = curl_easy_perform(curl_handle);
    // Need to reset cURL handler after that

    if (response != CURLE_OK) {
        FLB_LOG_ERROR("Can't fetch %s: %s", url, curl_easy_strerror(response));
        free(memory.data);
        return 1;
    }

    insert_comments(memory.data, context);

    free(memory.data);
    free(post_fields);
    return 0;
}
