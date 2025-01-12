//
//  tree.c
//  vulkanRenderer
//
//  Created by Markus Höglin on 2023-11-21.
//

#include "tree.h"

struct node {
    struct node *left;
    struct node *right;
    uint32_t value;
};

struct tree {
    tree_node *root;
    uint32_t size;
};

/* Node implementation*/

static inline int node_alloc(tree_node **node, uint32_t value) {
    tree_node *node_new = malloc(sizeof(tree_node));
    if(node_new == NULL) {
        return -1;
    } else {
        *node_new = (tree_node){
            .left = NULL,
            .right = NULL,
            .value = value
        };
        *node = node_new;
        return 0;
    }
}

static inline void node_free(tree_node *node) {
    if(node != NULL) {
        node_free(node->left);
        node_free(node->right);
        free(node);
    } 
}

static inline int node_add(tree_node *node, uint32_t value) {
    if(value < node->value) {
        if(node->left == NULL) {
            return node_alloc(&node->left, value);
        } else {
            return node_add(node->left, value);
        }
    } else if(value > node->value) {
        if(node->right == NULL) {
            return node_alloc(&node->right, value);
        } else {
            return node_add(node->right, value);
        }
    } else {
        return -1;
    }
}



uint32_t node_get_value(tree_node *node) {
    return node->value;
}

uint32_t node_get_depth(tree_node *node) {
    if(node == NULL) {
        return 0;
    } else {
        uint32_t left_depth = node_get_depth(node->left);
        uint32_t right_depth = node_get_depth(node->right);
        return (left_depth < right_depth) ? (right_depth + 1) : (left_depth + 1);
    }
}

tree_node *node_search(tree_node *node, uint32_t value) {
    return node;
}



static inline void node_ordered_print(tree_node *node) {
    if(node != NULL) {
        node_ordered_print(node->left);

        printf("%d\n", node->value);
        node_ordered_print(node->right);
    }
}

static inline void node_canonical_print(tree_node *node, uint32_t depth) {
    if(node == NULL) {
        for(uint32_t i = 0; i < depth; i++) {
            printf("-");
        }
        printf(">x\n");
    } else {
        for(uint32_t i = 0; i < depth; i++) {
            printf("-");
        }
        depth++;
        printf(">%d\n", node->value);
        printf("l:");
        node_canonical_print(node->left, depth);
        printf("r:");
        node_canonical_print(node->right, depth);
    }
}



void node_to_vector(tree_node *node, dynamic_vector *value_vector) {
    if(node != NULL) {
        node_to_vector(node->left, value_vector);
        vector_add(value_vector, &node->value);
        node_to_vector(node->right, value_vector);
    }
}

/* Tree implementation*/

int tree_alloc(binary_tree **tree) {
    binary_tree *tree_new = malloc(sizeof(binary_tree));
    if(tree_new == NULL) {
        return -1;
    } else {
        *tree_new = (binary_tree){
            .root = NULL,
            .size = 0
        };
        *tree = tree_new;
        return 0;
    }
}

void tree_free(binary_tree *tree) {
    node_free(tree->root);
    free(tree);
}

int tree_add(binary_tree *tree, uint32_t value) {
    int val_added;
    if(tree->root == NULL) {
        val_added = node_alloc(&tree->root, value);
    } else {
        val_added = node_add(tree->root, value);
    }
    
    if(val_added != -1) {
        tree->size++;
    }
    return val_added;
}



tree_node *tree_search(binary_tree *tree, uint32_t value) {
    return tree->root;
}

uint32_t tree_get_depth(binary_tree *tree) {
    if(tree->root != NULL) {
        return node_get_depth(tree->root);
    } else {
        return 0;
    }
}

uint32_t tree_get_size(binary_tree *tree) {
    return tree->size;
}



void tree_ordered_print(binary_tree *tree) {
    if(tree->root != NULL) {
        node_ordered_print(tree->root);
    }
}

void tree_canonical_print(binary_tree *tree) {
    if(tree->root != NULL) {
        node_canonical_print(tree->root, 0);
    }
}



void tree_to_vector(binary_tree *tree, dynamic_vector *value_vector) {
    if(tree->root != NULL) {
        node_to_vector(tree->root, value_vector);
    }
}