// Copyright Ian R.Shehadeh 2018

#include <string.h>
#include <stdlib.h>
#include "simplify/rbtree/rbtree.h"


static inline rbtree_node_t* rbtree_grandparent(rbtree_node_t* node) {
    return node->parent->parent;
}

static inline rbtree_node_t* rbtree_uncle(rbtree_node_t* node) {
    rbtree_node_t* gp = rbtree_grandparent(node);
    if (gp->left == node->parent) {
        return gp->right;
    }

    return gp->left;
}

static inline void rbtree_rotate_left(rbtree_t* tree, rbtree_node_t* node) {
    rbtree_node_t* right = node->right;
    node->right = node->left;
    if (node->right)
        node->right->parent = node;
    right->parent = node->parent;

    if (!node->parent)
        tree->root = right;
    else if (node == node->parent->left)
        node->parent->left = right;
    else
        node->parent->right = right;
    right->left = node;
    node->parent = right;
}

static inline void rbtree_rotate_right(rbtree_t* tree, rbtree_node_t* node) {
    rbtree_node_t* left = node->left;
    node->left = node->right;
    if (node->left)
        node->left->parent = node;
    left->parent = node->parent;

    if (!node->parent)
        tree->root = left;
    else if (node == node->parent->right)
        node->parent->right = left;
    else
        node->parent->right = left;
    left->right = node;
    node->parent = left;
}

void rbtree_free_node_recursive(rbtree_node_t* node, void(*free_func)(void*)) {
    free(node->key);
    free_func(node->data);
    if (node->left)
        rbtree_free_node_recursive(node->left, free_func);

    if (node->right)
        rbtree_free_node_recursive(node->right, free_func);
    free(node);
}

error_t rbtree_basic_insert(rbtree_t* tree, rbtree_node_t* node) {
    if (!tree->root) {
        tree->root = node;
        node->parent = NULL;
        return ERROR_NO_ERROR;
    }

    rbtree_node_t* current = tree->root;
    while (1) {
        int result = strcmp(node->key, current->key);
        if (result == 0) {
            current->data = node->data;
            return ERROR_NO_ERROR;
        } else if (result > 0) {
            if (current->left) {
                current = current->left;
            } else {
                current->left = node;
                break;
            }
        } else {
            if (current->right) {
                current = current->right;
            } else {
                current->right = node;
                break;
            }
        }
    }
    node->parent = current;

    return ERROR_NO_ERROR;
}

error_t rbtree_search(rbtree_t* tree, char* key, void** dataout) {
    if (!tree->root)
        return ERROR_NONEXISTANT_KEY;

    rbtree_node_t* current = tree->root;
    while (1) {
        int result = strcmp(key, current->key);
        if (result == 0) {
            *dataout = current->data;
            return ERROR_NO_ERROR;
        } else if (result > 0) {
            if (current->left)
                current = current->left;
            else
                return ERROR_NONEXISTANT_KEY;
        } else {
            if (current->right)
                current = current->right;
            else
                return ERROR_NONEXISTANT_KEY;
        }
    }

    // UNREACHABLE!
}

void rbtree_clean(rbtree_t* tree, void(*free_func)(void*)) {
    if (tree->root)
        rbtree_free_node_recursive(tree->root, free_func);
}

error_t rbtree_insert(rbtree_t* tree, char* key, void* value) {
    rbtree_node_t* node = malloc(sizeof(rbtree_node_t));
    if (!node) return ERROR_FAILED_TO_ALLOCATE;

    size_t key_len = strlen(key);
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->key = malloc(key_len + 1);
    node->color = RBTREE_COLOR_RED;
    node->data = value;

    node->key[key_len] = 0;
    strncpy(node->key, key, key_len);

    error_t err = rbtree_basic_insert(tree, node);
    if (err) return err;

    if (!node->parent) {
        node->color = RBTREE_COLOR_BLACK;
        return ERROR_NO_ERROR;
    }

    if (node->parent->color != RBTREE_COLOR_BLACK) {
        switch (rbtree_uncle(node)->color) {
            case RBTREE_COLOR_RED:
            {
                // if the uncle is red we recolor

                rbtree_uncle(node)->color = RBTREE_COLOR_BLACK;
                rbtree_grandparent(node)->color = RBTREE_COLOR_RED;
                node->parent->color = RBTREE_COLOR_BLACK;

                rbtree_node_t* gp = rbtree_grandparent(node);
                rbtree_uncle(gp)->color = RBTREE_COLOR_BLACK;
                rbtree_grandparent(gp)->color = RBTREE_COLOR_RED;
                gp->parent->color = RBTREE_COLOR_BLACK;
                break;
            }
            case RBTREE_COLOR_BLACK:
            {
                // if the uncle is black we rotate

                if (node->parent->parent->left == node->parent) {
                    if (node->parent->right == node)
                        rbtree_rotate_left(tree, node->parent);
                    rbtree_rotate_right(tree, node->parent->parent);
                } else {
                    if (node->parent->right == node)
                        rbtree_rotate_right(tree, node->parent);
                    rbtree_rotate_left(tree, node->parent->parent);
                }
                break;
            }
        }
    }
    if (node->parent->parent) {
        rbtree_color_t pcolor = node->parent->color;
        node->parent->color = node->parent->parent->color;
        node->parent->parent->color = pcolor;
    }
    return ERROR_NO_ERROR;
}
