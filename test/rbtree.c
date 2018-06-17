/* Copyright Ian Shehadeh 2018 */

#include "simplify/rbtree/rbtree.h"
#include "test/test.h"

#define ITERATE_PAIRS(FIRST, SECOND) \
    FIRST = __key_list[0];           \
    SECOND = __value_list[0];        \
    for (int i = 0; i < 36; ++i, FIRST = __key_list[i], SECOND = __value_list[i])

static char* __key_list[] = {"Peter",
                             "Piper",
                             "picked",
                             "a peck",
                             "of pickled",
                             "peppers",
                             "A peck",
                             "of pickled ",
                             "peppers Peter",
                             "Piper picked",
                             "If Peter",
                             "Piper picked ",
                             "a peck of",
                             "pickled peppers",
                             "Where's the peck of",
                             "pickled peppers Peter Piper",
                             "picked?",
                             "I saw",
                             "Susie sitting"
                             "in a shoe shine shop",
                             "Where she sits she",
                             "shines,",
                             "and where she",
                             "shines",
                             "she",
                             "sits.",
                             "How can",
                             "a clam",
                             " cram in a clean",
                             "cream can?",
                             "Coy",
                             "knows",
                             "pseudonoise",
                             "codes.",
                             "Six sick hicks nick",
                             "six slick bricks",
                             "with picks",
                             "and"};

static char* __value_list[] = {
    "sticks.",
    "To sit",
    "in solemn silence in a dull",
    "dark dock",
    "In a pestilential",
    "prison, with",
    "a life-long lock",
    "Awaiting the",
    " sensation of a short",
    "sharp shock",
    "From a cheap ",
    "and chippy chopper on",
    "a big black block!",
    "To sit in solemn",
    " silence in a dull",
    ", dark dock,",
    "In a pestilential prison,",
    "with a life",
    "-long lock",
    "Awaiting the sensation",
    "of a short",
    ", sharp shock",
    "From a cheap and chippy",
    "chopper on a big black block!",
    "A dull,",
    "dark dock, a",
    "life-long lock,",
    "A short",
    ", sharp shock, a",
    "big black block!",
    "To sit in solemn silence",
    "in a pestilential prison,",
    "And awaiting the",
    "sensation",
    "From a cheap",
    " and chippy chopper",
    "on a big black block!",
};

int main() {
    rbtree_t tree;
    error_t err;
    rbtree_init(&tree);

    char* key;
    char* value;
    ITERATE_PAIRS(key, value) {
        err = rbtree_insert(&tree, key, value);
        if (err) {
            FATAL("(iteration %d) %s", i, error_string(err));
        }
    }

    ITERATE_PAIRS(key, value) {
        char* data;
        err = rbtree_search(&tree, key, (void**)&data);
        if (err) {
            FATAL("(iteration %d) \"%s\"", i, error_string(err));
        }
        if (data != value) {
            FATAL("(iteration %d) data did not match: expected \"%s\", got \"%s\"", i, value, data);
        }
    }

    rbtree_clean(&tree, NULL);
}
