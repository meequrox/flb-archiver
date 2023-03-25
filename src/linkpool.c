#include "linkpool.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LinkNode* linknode_create(char* url, char* filename) {
    LinkNode* node = (LinkNode*)malloc(sizeof(LinkNode));
    if (node) {
        node->url = malloc(strlen(url) * sizeof(char));
        node->filename = malloc(strlen(filename) * sizeof(char));
        strncpy(node->url, url, strlen(url) + 1);
        strncpy(node->filename, filename, strlen(filename) + 1);

        node->next = NULL;
    } else
        fprintf(stderr, "%s: malloc failed\n", __FUNCTION__);

    return node;
}

LinkPool* linkpool_create(void) {
    LinkPool* pool = (LinkPool*)malloc(sizeof(LinkPool));
    if (pool)
        pool->head = NULL;
    else
        fprintf(stderr, "%s: malloc failed\n", __FUNCTION__);

    return pool;
}

void linkpool_print(LinkPool* pool) {
    if (pool) {
        LinkNode* cur = pool->head;
        while (cur) {
            fprintf(stdout, "%s -> %s\n", cur->url, cur->filename);
            cur = cur->next;
        }
    }
}

void linkpool_clear(LinkPool* pool) {
    if (pool) {
        LinkNode* cur = pool->head;

        while (cur) {
            LinkNode* prev = cur;

            cur = cur->next;
            free(prev->url);
            free(prev->filename);
            free(prev);
        }

        pool->head = NULL;
    }
}

void linkpool_free(LinkPool* pool) {
    linkpool_clear(pool);
    free(pool);
}

LinkPool* linkpool_push_node(LinkPool* pool, char* url, char* filename) {
    if (!pool) return NULL;

    LinkNode* cur = pool->head;
    LinkNode* prev = NULL;

    while (cur) {
        if (strcmp(url, cur->url) == 0 && strcmp(filename, cur->filename) == 0) return pool;

        prev = cur;
        cur = cur->next;
    }

    LinkNode* node = linknode_create(url, filename);
    if (prev)
        prev->next = node;
    else
        pool->head = node;

    return pool;
}

LinkPool* linkpool_delete_node(LinkPool* pool, char* url, char* filename) {
    if (!pool) return NULL;

    LinkNode* cur = pool->head;
    LinkNode* prev = NULL;

    while (cur) {
        if (strcmp(url, cur->url) == 0 && strcmp(filename, cur->filename) == 0) {
            if (prev)
                prev->next = cur->next;
            else
                pool->head = cur->next;

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
