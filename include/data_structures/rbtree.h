#ifndef FLB_ARCHIVER_RBTREE_H
#define FLB_ARCHIVER_RBTREE_H

enum flb_rbtree_colors { FLB_RED, FLB_BLACK };

typedef struct flb_rbnode_t {
    struct flb_rbnode_t* parent;
    struct flb_rbnode_t* left;
    struct flb_rbnode_t* right;

    char* key;
    char* value;
    int color;
} flb_rbnode;

typedef struct flb_rbtree_t {
    flb_rbnode* root;
    flb_rbnode* leaf;
} flb_rbtree;

flb_rbtree* flb_rbtree_create(void);

flb_rbnode* flb_rbtree_insert(flb_rbtree* tree, const char* key, const char* value);
flb_rbnode* flb_rbtree_lookup(const flb_rbtree* tree, const char* key);
flb_rbnode* flb_rbtree_delete(flb_rbtree* tree, const char* key);

void flb_rbtree_free(const flb_rbtree* tree, flb_rbnode* root);

#endif  // FLB_ARCHIVER_RBTREE_H
