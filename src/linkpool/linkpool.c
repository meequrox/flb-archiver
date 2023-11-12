#include "linkpool/linkpool.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LinkNode* linknode_create(const char* url, const char* filename) {
    LinkNode* node = (LinkNode*) malloc(sizeof(LinkNode));

    if (!node) {
        fprintf(stderr, "%s: malloc failed\n", __func__);
        return NULL;
    }

    node->url = malloc((strlen(url) + 1) * sizeof(char));
    node->filename = malloc((strlen(filename) + 1) * sizeof(char));
    strncpy(node->url, url, strlen(url) + 1);
    strncpy(node->filename, filename, strlen(filename) + 1);

    node->next = NULL;

    return node;
}

LinkPool* linkpool_create(void) {
    LinkPool* pool = (LinkPool*) malloc(sizeof(LinkPool));

    if (pool) {
        pool->head = NULL;
    } else {
        fprintf(stderr, "%s: malloc failed\n", __func__);
    }

    return pool;
}

void linkpool_print(FILE* stream, const LinkPool* pool) {
    if (!stream) {
        stream = stdout;
    }

    if (pool) {
        LinkNode* cur = pool->head;
        while (cur) {
            fprintf(stream, "%s -> %s\n", cur->url, cur->filename);
            cur = cur->next;
        }
    }
}

static void linknode_clearlist(LinkNode* node) {
    LinkNode* cur = NULL;

    while (node) {
        cur = node;
        node = node->next;

        free(cur->filename);
        free(cur->url);
        cur->filename = NULL;
        cur->url = NULL;

        free(cur);
    }
}

void linkpool_clear(LinkPool* pool) {
    if (pool) {
        linknode_clearlist(pool->head);
        pool->head = NULL;
    }
}

void linkpool_free(LinkPool* pool) {
    if (pool) {
        linkpool_clear(pool);

        free(pool);
        pool = NULL;
    }
}

LinkPool* linkpool_push_node(LinkPool* pool, const char* url, const char* filename) {
    if (!pool) {
        return NULL;
    }

    LinkNode* cur = pool->head;
    LinkNode* prev = NULL;

    while (cur) {
        if (strcmp(url, cur->url) == 0 && strcmp(filename, cur->filename) == 0) {
            return pool;
        }

        prev = cur;
        cur = cur->next;
    }

    LinkNode* node = linknode_create(url, filename);
    if (prev) {
        prev->next = node;
    } else {
        pool->head = node;
    }

    return pool;
}

LinkPool* linkpool_delete_node(LinkPool* pool, const char* url, const char* filename) {
    if (!pool) {
        return NULL;
    }

    LinkNode* cur = pool->head;
    LinkNode* prev = NULL;

    while (cur) {
        if (strcmp(url, cur->url) == 0 && strcmp(filename, cur->filename) == 0) {
            if (prev) {
                prev->next = cur->next;
            } else {
                pool->head = cur->next;
            }

            free(cur->url);
            free(cur->filename);
            free(cur);
            break;
        }

        prev = cur;
        cur = cur->next;
    }

    return pool;
}
