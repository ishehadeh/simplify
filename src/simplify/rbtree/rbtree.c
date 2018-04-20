/* Copyright Ian Shehadeh 2018 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "simplify/rbtree/rbtree.h"


static inline rbtree_node_t* rbtree_grandparent(rbtree_node_t* node) {
    return node->parent != NULL ? node->parent->parent : NULL;
}

static inline rbtree_node_t* rbtree_uncle(rbtree_node_t* node) {
    rbtree_node_t* gp = rbtree_grandparent(node);
    if (!gp) return NULL;
    return gp->left == node->parent ? gp->right : gp->left;
}

static inline void rbtree_rotate_left(rbtree_t* tree, rbtree_node_t* node) {
    rbtree_node_t* right = node->right;

    // Swap node's right subtree with it's child
    node->right = node->right->left;

    // if right's left child is not root, make it's parent node.
    if (right->left)
        right->left->parent = node;

    right->parent = node->parent;

    if (!node->parent)
        // since root doesn't have an uncle there is a special case
        tree->root = right;

    // otherwise swap the uncle with node's old left child
    else if (node == node->parent->left)
        node->parent->left = right;
    else
        node->parent->right = right;

    right->left = node;
    node->parent = right;
}

static inline void rbtree_rotate_right(rbtree_t* tree, rbtree_node_t* node) {
    rbtree_node_t* left = node->left;

    // Swap node's left subtree with it's child
    node->left = node->left->right;

    // if right's right child is not root, make it's parent node.
    if (left->right)
        left->right->parent = node;

    left->parent = node->parent;

    if (!node->parent)
        // since root doesn't have an uncle there is a special case
        tree->root = left;

    // otherwise swap the uncle with node's old right child
    else if (node == node->parent->right)
        node->parent->right = left;
    else
        node->parent->left = left;

    left->right = node;
    node->parent = left;
}

void rbtree_free_node_recursive(rbtree_node_t* node, void(*free_func)(void*)) {
    free(node->key);
    if (free_func)
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
    while (current) {
        int result = strcmp(key, current->key);
        if (result == 0) {
            *dataout = current->data;
            return ERROR_NO_ERROR;
        } else if (result > 0) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    return ERROR_NONEXISTANT_KEY;
}

void rbtree_clean(rbtree_t* tree, void(*free_func)(void*)) {
    if (tree->root)
        rbtree_free_node_recursive(tree->root, free_func);
}

error_t rbtree_balance(rbtree_t* tree, rbtree_node_t* node) {
    rbtree_node_t* gp;
    rbtree_node_t* uncle;
    while (node != tree->root && node->parent->color == RBTREE_COLOR_RED) {
        gp = rbtree_grandparent(node);
        if (!gp)
            break;
        uncle = rbtree_uncle(node);

        if (uncle && uncle->color == RBTREE_COLOR_RED) {
            gp->color = RBTREE_COLOR_RED;
            node->parent->color = RBTREE_COLOR_BLACK;
            uncle->color = RBTREE_COLOR_BLACK;
            node = gp;
        } else {
            if (gp->left == node->parent) {
                if (node == node->parent->right) {
                    rbtree_rotate_left(tree, node->parent);
                }

                rbtree_rotate_right(tree, gp);
            } else if (gp->right == node->parent)  {
                if (node == node->parent->left) {
                    rbtree_rotate_right(tree, node->parent);
                }

                rbtree_rotate_left(tree, gp);
            }
            if (node->parent) {
                rbtree_color_t t = node->parent->color;
                node->parent->color = gp->color;
                gp->color = t;
            }
        }
    }
    tree->root->color = RBTREE_COLOR_BLACK;
    return ERROR_NO_ERROR;
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

    return rbtree_balance(tree, node);
}
