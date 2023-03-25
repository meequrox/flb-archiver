#ifndef LINKPOOL_H
#define LINKPOOL_H

typedef struct linknode {
    char* url;
    char* filename;
    struct linknode* next;
} LinkNode;

typedef struct linkpool {
    LinkNode* head;
} LinkPool;

LinkPool* linkpool_create(void);
void linkpool_print(LinkPool* pool);
void linkpool_clear(LinkPool* pool);
void linkpool_free(LinkPool* pool);

LinkPool* linkpool_push_node(LinkPool* pool, char* url, char* filename);
LinkPool* linkpool_delete_node(LinkPool* pool, char* url, char* filename);

#endif  // LINKPOOL_H
