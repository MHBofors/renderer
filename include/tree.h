//
//  tree.h
//  vulkanRenderer
//
//  Created by Markus Höglin on 2023-11-21.
//

#ifndef tree_h
#define tree_h

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <dynamic_vector.h>

typedef struct node tree_node;
typedef struct tree binary_tree;

static inline int node_alloc(tree_node **node, uint32_t value);
static inline void node_free(tree_node *node);
static inline int node_add(tree_node *node, uint32_t value);

uint32_t node_get_value(tree_node *node);
uint32_t node_get_depth(tree_node *node);
tree_node *node_search(tree_node *node, uint32_t value);

static inline void node_ordered_print(tree_node *node);
static inline void node_canonical_print(tree_node *node, uint32_t depth);

void node_to_vector(tree_node *node, dynamic_vector *value_vector);

int tree_alloc(binary_tree **tree);
void tree_free(binary_tree *tree);
int tree_add(binary_tree *tree, uint32_t value);

uint32_t tree_get_size(binary_tree *tree);
uint32_t tree_get_depth(binary_tree *tree);
tree_node *tree_search(binary_tree *tree, uint32_t value);

void tree_ordered_print(binary_tree *tree);
void tree_canonical_print(binary_tree *tree);

void tree_to_vector(binary_tree *tree, dynamic_vector *value_vector);

#endif