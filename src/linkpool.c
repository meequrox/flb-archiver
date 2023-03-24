#include "linkpool.h"

#include <stdio.h>
#include <stdlib.h>

size_t linkpool_nodes = 0;
int linkpool_freed = 0;

void linkpool_print(struct LinkPoolNode** pool) {
    if (!pool) {
        fprintf(stderr, "%s: link pool is NULL\n", __FUNCTION__);
        return;
    }

    for (int i = 0; i < linkpool_nodes && pool[i] && pool[i]->exist; i++) {
        fprintf(stdout, "%s - %s\n", pool[i]->url, pool[i]->filename);
    }
}

void linkpool_free(struct LinkPoolNode** pool) {
    if (!pool || linkpool_freed) return;

    for (int i = 0; i < linkpool_nodes && pool[i]; i++) {
        free(pool[i]->url);
        free(pool[i]->filename);
        free(pool[i]);
    }

    free(pool);
    linkpool_freed = 1;
}
