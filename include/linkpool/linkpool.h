#ifndef FLB_ARCHIVER_LINKPOOL_H
#define FLB_ARCHIVER_LINKPOOL_H

typedef struct linknode {
    char* url;
    char* filename;
    struct linknode* next;
} LinkNode;

typedef struct linkpool {
    LinkNode* head;
} LinkPool;

LinkPool* linkpool_create(void);
void linkpool_print(const LinkPool* pool);
void linkpool_clear(LinkPool* pool);
void linkpool_free(LinkPool* pool);

LinkPool* linkpool_push_node(LinkPool* pool, const char* url, const char* filename);
LinkPool* linkpool_delete_node(LinkPool* pool, const char* url, const char* filename);

#endif  // FLB_ARCHIVER_LINKPOOL_H
