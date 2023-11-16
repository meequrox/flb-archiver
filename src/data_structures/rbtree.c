#include "data_structures/rbtree.h"

#include <stdlib.h>
#include <string.h>

flb_rbtree* flb_rbtree_create(void) {
    flb_rbtree* tree = (flb_rbtree*) malloc(sizeof(flb_rbtree));
    if (!tree) {
        return NULL;
    }

    tree->leaf = (flb_rbnode*) malloc(sizeof(flb_rbnode));

    if (!tree->leaf) {
        free(tree);
        return NULL;
    }

    tree->root = NULL;

    tree->leaf->parent = NULL;
    tree->leaf->left = NULL;
    tree->leaf->right = NULL;
    tree->leaf->color = kColorBlack;

    tree->leaf->key = NULL;
    tree->leaf->value = NULL;

    return tree;
}

static void rbtree_rotate_left(const flb_rbtree* tree, flb_rbnode* node) {
    flb_rbnode* rchild = node->right;

    node->right = rchild->left;
    if (rchild->left != tree->leaf) {
        rchild->left->parent = node;
    }

    rchild->parent = node->parent;

    if (rchild->parent) {
        if (node == node->parent->left) {
            node->parent->left = rchild;
        } else {
            node->parent->right = rchild;
        }
    }

    rchild->left = node;
    node->parent = rchild;
}

static void rbtree_rotate_right(const flb_rbtree* tree, flb_rbnode* node) {
    flb_rbnode* lchild = node->left;

    node->left = lchild->right;
    if (lchild->right != tree->leaf) {
        lchild->right->parent = node;
    }

    lchild->parent = node->parent;

    if (lchild->parent) {
        if (node == node->parent->left) {
            node->parent->left = lchild;
        } else {
            node->parent->right = lchild;
        }
    }

    lchild->right = node;
    node->parent = lchild;
}

static flb_rbnode* rbtree_add_fixup(flb_rbtree* tree, flb_rbnode* node) {
    flb_rbnode* uncle = NULL;

    while (node->parent && node->parent->color == kColorRed) {
        if (node->parent == node->parent->parent->left) {
            uncle = node->parent->parent->right;

            if (uncle->color == kColorRed) {
                node->parent->color = kColorBlack;
                node->parent->parent->color = kColorRed;
                uncle->color = kColorBlack;

                node = node->parent->parent;
            } else {
                if (node == node->parent->right) {
                    node = node->parent;
                    rbtree_rotate_left(tree, node);
                }

                node->parent->color = kColorBlack;
                node->parent->parent->color = kColorRed;

                if (node->parent->parent == tree->root) {
                    tree->root = node->parent->parent->left;
                }

                rbtree_rotate_right(tree, node->parent->parent);
            }
        } else {
            uncle = node->parent->parent->left;

            if (uncle->color == kColorRed) {
                node->parent->color = kColorBlack;
                node->parent->parent->color = kColorRed;
                uncle->color = kColorBlack;

                node = node->parent->parent;
            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    rbtree_rotate_right(tree, node);
                }

                node->parent->color = kColorBlack;
                node->parent->parent->color = kColorRed;

                if (node->parent->parent == tree->root) {
                    tree->root = node->parent->parent->right;
                }

                rbtree_rotate_left(tree, node->parent->parent);
            }
        }
    }

    tree->root->color = kColorBlack;

    return node;
}

static flb_rbnode* rbtree_create_node(const flb_rbtree* tree, const char* key, const char* value) {
    flb_rbnode* node = malloc(sizeof(flb_rbnode));
    char* key_copy = strdup(key);
    char* value_copy = strdup(value);

    if (!node || !key_copy || !value_copy) {
        free(node);
        free(key_copy);
        free(value_copy);
        return NULL;
    }

    node->parent = NULL;
    node->left = tree->leaf;
    node->right = tree->leaf;

    node->key = key_copy;
    node->value = value_copy;

    node->color = kColorRed;

    return node;
}

flb_rbnode* flb_rbtree_insert(flb_rbtree* tree, const char* key, const char* value) {
    if (!tree || !key) {
        return NULL;
    }

    flb_rbnode* tmp = tree->root;
    flb_rbnode* parent = tmp;

    while (tmp && tmp != tree->leaf) {
        parent = tmp;

        if (strcmp(key, tmp->key) < 0) {
            tmp = tmp->left;
        } else if (strcmp(key, tmp->key) > 0) {
            tmp = tmp->right;
        } else {
            return tmp;
        }
    }

    flb_rbnode* node = rbtree_create_node(tree, key, value);
    if (!node) {
        return NULL;
    }

    if (!tree->root) {
        tree->root = node;
        node->color = kColorBlack;
    } else {
        if (strcmp(key, parent->key) < 0) {
            parent->left = node;
        } else {
            parent->right = node;
        }
    }

    node->parent = parent;

    if (parent && parent->color == kColorRed) {
        rbtree_add_fixup(tree, node);
    }

    return node;
}

flb_rbnode* flb_rbtree_lookup(const flb_rbtree* tree, const char* key) {
    if (!tree || !tree->root || !key) {
        return NULL;
    }

    flb_rbnode* node = tree->root;

    while (node != tree->leaf) {
        if (strcmp(key, node->key) < 0) {
            node = node->left;
        } else if (strcmp(key, node->key) > 0) {
            node = node->right;
        } else {
            return node;
        }
    }

    return NULL;
}

static void rbtree_delete_fixup(flb_rbtree* tree, flb_rbnode* node) {
    if (node == tree->leaf) {
        return;
    }

    flb_rbnode* tmp_node = NULL;

    while (node != tree->root && node->color == kColorBlack) {
        if (node == node->parent->left) {
            tmp_node = node->parent->right;

            if (tmp_node->color == kColorRed) {
                tmp_node->color = kColorBlack;
                node->parent->color = kColorRed;
                rbtree_rotate_left(tree, node->parent);
                tmp_node = node->parent->right;
            }

            if (tmp_node->left->color == kColorBlack && tmp_node->right->color == kColorBlack) {
                tmp_node->color = kColorRed;
                node = node->parent;
            } else if (tmp_node->right->color == kColorBlack) {
                tmp_node->left->color = kColorBlack;
                tmp_node->color = kColorRed;
                rbtree_rotate_right(tree, tmp_node);
                tmp_node = node->parent->right;
            }

            tmp_node->color = node->parent->color;
            node->parent->color = kColorBlack;
            tmp_node->right->color = kColorBlack;
            rbtree_rotate_left(tree, node->parent);

            node = tree->root;
        } else {
            tmp_node = node->parent->left;

            if (tmp_node->color == kColorRed) {
                tmp_node->color = kColorBlack;
                node->parent->color = kColorRed;
                rbtree_rotate_right(tree, node->parent);
                tmp_node = node->parent->left;
            }

            if (tmp_node->left->color == kColorBlack && tmp_node->right->color == kColorBlack) {
                tmp_node->color = kColorRed;
                node = node->parent;
            } else if (tmp_node->left->color == kColorBlack) {
                tmp_node->right->color = kColorBlack;
                tmp_node->color = kColorRed;
                rbtree_rotate_left(tree, tmp_node);
                tmp_node = node->parent->left;
            }

            tmp_node->color = node->parent->color;
            node->parent->color = kColorBlack;
            tmp_node->left->color = kColorBlack;
            rbtree_rotate_right(tree, node->parent);
            node = tree->root;
        }
    }

    node->color = kColorBlack;
}

static void rbtree_transplant(flb_rbtree* tree, flb_rbnode* node, flb_rbnode* new_node) {
    if (!node->parent || node->parent == tree->leaf) {
        tree->root = new_node;
    } else if (node == node->parent->left) {
        node->parent->left = new_node;
    } else {
        node->parent->right = new_node;
    }

    if (new_node != tree->leaf) {
        new_node->parent = node->parent;
    }
}

flb_rbnode* flb_rbtree_delete(flb_rbtree* tree, const char* key) {
    if (!tree || !tree->root) {
        return NULL;
    }

    if (!key) {
        return tree->root;
    }

    flb_rbnode* node = flb_rbtree_lookup(tree, key);
    flb_rbnode* new_node = node;

    flb_rbnode* tmp = NULL;

    int new_node_color = new_node->color;

    if (node->left == tree->leaf) {
        tmp = node->right;
        rbtree_transplant(tree, node, node->right);
    } else if (node->right == tree->leaf) {
        tmp = node->left;
        rbtree_transplant(tree, node, node->left);
    } else {
        flb_rbnode* min_node = node->right;

        while (min_node->left != tree->leaf) {
            min_node = min_node->left;
        }

        new_node = min_node;
        new_node_color = new_node->color;

        tmp = new_node->right;

        if (new_node->parent == node) {
            tmp->parent = new_node;
        } else {
            rbtree_transplant(tree, new_node, new_node->right);

            new_node->right = node->right;
            new_node->right->parent = new_node;
        }

        rbtree_transplant(tree, node, new_node);

        new_node->left = node->left;
        new_node->left->parent = new_node;
        new_node->color = node->color;
    }

    if (new_node_color == kColorBlack) {
        rbtree_delete_fixup(tree, tmp);
    }

    tree->root->color = kColorBlack;

    return new_node;
}

static void rbtree_free_nodes(const flb_rbtree* tree, flb_rbnode* root) {
    if (!root || root == tree->leaf) {
        return;
    }

    rbtree_free_nodes(tree, root->left);
    rbtree_free_nodes(tree, root->right);

    free(root->key);
    free(root->value);

    // Safe free
    root->key = NULL;
    root->value = NULL;
    root->parent = NULL;
    root->left = NULL;
    root->right = NULL;
    free(root);
}

void flb_rbtree_free(flb_rbtree* tree) {
    if (!tree) {
        return;
    }

    if (tree->root) {
        rbtree_free_nodes(tree, tree->root);
        tree->root = NULL;
    }

    free(tree->leaf);
    tree->leaf = NULL;

    free(tree);
}

static int rbtree_foreach3(const flb_rbtree* tree, const flb_rbnode* root,
                           int (*fun)(void*, const char*, const char*), void* first_arg) {
    if (!root || root == tree->leaf) {
        return 0;
    }

    int rc = 0;

    // DFS
    rc += rbtree_foreach3(tree, root->left, fun, first_arg);
    rc += rbtree_foreach3(tree, root->right, fun, first_arg);

    rc += fun(first_arg, root->key, root->value);
    return rc;
}

int flb_rbtree_foreach3(const flb_rbtree* tree, int (*fun)(void*, const char*, const char*),
                        void* first_arg) {
    int rc = 1;

    if (tree && fun) {
        rc = rbtree_foreach3(tree, tree->root, fun, first_arg);
    }

    return rc;
}
