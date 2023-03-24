#ifndef LINKPOOL_H
#define LINKPOOL_H

#include <stddef.h>

#define LINKPOOL_MAX_LINKS 30

extern size_t linkpool_nodes;
extern int linkpool_freed;

struct LinkPoolNode {
    char* url;
    char* filename;
    int exist;
};

void linkpool_print(struct LinkPoolNode** pool);
void linkpool_free(struct LinkPoolNode** pool);

#endif  // LINKPOOL_H
