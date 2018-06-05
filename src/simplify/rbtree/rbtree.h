/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_RBTREE_RBTREE_H_
#define SIMPLIFY_RBTREE_RBTREE_H_


#include <stddef.h>
#include "simplify/errors.h"

/* default to 2KB for RBTREE_CHUNK_SIZE 
    This gaurentees the all the builtins can fit in one chunk
*/
#if !defined(RBTREE_CHUNK_SIZE) && defined(RBTREE_USE_CHUNKS)
#   define RBTREE_CHUNK_SIZE 2048
#endif

/* A node in the Red-Black Tree
 */
typedef struct rbtree_node  rbtree_node_t;

/* A Red-Black Tree
 *
 * A Red-Black Tree is a type of self-balancing binary search tree.
 */
typedef struct rbtree       rbtree_t;

/* Either Red, or Black
 */
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

#if defined(RBTREE_USE_CHUNKS)
    struct rbtree_chunk {
        void*        data;
        size_t       used;
        struct rbtree_chunk* last;
    };

    struct rbtree {
        /* to avoid allocations allocate rbtree memory to the slab */
        struct rbtree_chunk* slab;
        struct rbtree_node* root;
    };
#else
    struct rbtree {
        struct rbtree_node* root;
    };
#endif

/* insert a new item into the tree
 *
 * @tree the tree to insert into
 * @key the key to assign to `data`
 * @data the data to insert into the tree
 * @return an error code
 */
error_t rbtree_insert(rbtree_t* tree, char* key, void* data);

/* get an item from the tree
 *
 * @tree the tree to search
 * @key the key to look for
 * @out_data a pointer to a pointer that will be assigned the pointer passed into the tree with rbtree_insert
 * @return an error is returned if the key is not found, and `data_out` is not assigned to.
 */
error_t rbtree_search(rbtree_t* tree, char* key, void** data_out);

/* free all resources from a tree (both keys and data)
 *
 * @tree the tree to clean
 * @data_cleaner a function to clean up the tree's data
 */
void    rbtree_clean(rbtree_t* tree, void(*data_cleaner)(void*));

/* initialize an rbtree
 *
 * @tree the tree to initialize
 */
static inline void rbtree_init(rbtree_t* tree) {
    tree->root = NULL;

#if defined(RBTREE_USE_CHUNKS)
    tree->slab = NULL;
#endif
}

#endif  // SIMPLIFY_RBTREE_RBTREE_H_
