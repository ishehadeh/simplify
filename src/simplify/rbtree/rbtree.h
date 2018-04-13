// Copyright Ian R. Shehadeh 2018

#ifndef SIMPLIFY_RBTREE_RBTREE_H_
#define SIMPLIFY_RBTREE_RBTREE_H_

#include "simplify/errors.h"

typedef struct rbtree_node  rbtree_node_t;
typedef struct rbtree       rbtree_t;
typedef enum   rbtree_color rbtree_color_t;


enum rbtree_color {
    RBTREE_COLOR_RED,
    RBTREE_COLOR_BLACK,
};

struct rbtree_node {
    rbtree_color_t color;

    struct rbtree_node* left;
    struct rbtree_node* right;
    struct rbtree_node* parent;

    void* data;
    char* key;
};

struct rbtree {
    struct rbtree_node* root;
};

error_t rbtree_insert(rbtree_t*, char*, void*);
error_t rbtree_search(rbtree_t*, char*, void**);
void    rbtree_clean(rbtree_t*, void(*)(void*));

static inline void rbtree_init(rbtree_t* tree) {
    tree->root = NULL;
}

#endif  // SIMPLIFY_RBTREE_RBTREE_H_
