#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef REDBLACK
#define REDBLACK
#define TOPLEVEL
#endif

#ifndef VALUE
#define VALUE int
#define VALUE_FORMAT "%d"
#endif

#ifndef INVALID
#define INVALID -1
#endif

#include "common.c"

VALUE find(VALUE value, Node** root, bool insert){
    // if insert, insert if not found (silently just return if found either way)

    // Handle empty tree separately
    if (!*root){
        if (insert){
            Node* new = _allocated_memory ? get_node() : malloc(sizeof(Node));
#ifdef _TOUCH
            new->version = 0;
#endif
            new->right = NULL;
            new->left = NULL;
            new->value = value;
            new->blackness = 0;
            new->black_height = 0;

            *root = new;
            return value;
        } else {
            return INVALID;
        }
    }

    node_stack stack;
    init_node_stack(&stack, 8);
    Node* current_node = *root;

    // find
    while(current_node && TOUCH(current_node)->value != value){
        node_push(&stack, current_node);
        if (TOUCH(current_node)->value > value){
            current_node = TOUCH(current_node)->left;
        } else {
            current_node = TOUCH(current_node)->right;
        }
    }

    VALUE result = value;

    // handle insertion
    if (!current_node){
        current_node = node_pop(&stack);
        if (insert){
            Node* new = _allocated_memory ? get_node() : malloc(sizeof(Node));
#ifdef _TOUCH
            new->version = 0;
#endif
            new->right = NULL;
            new->left = NULL;
            new->value = value;
            new->blackness = 0;
            new->black_height = 0;
            if (TOUCH(current_node)->value > value){
                TOUCH(current_node)->left = new;
            } else {
                TOUCH(current_node)->right = new;
            }
            node_push(&stack, current_node);
            current_node = new;
        } else {
            result = TOUCH(current_node)->value;
            goto cleanup;
        }
    } else {
        goto cleanup;
    }

    rebalance_after_insert(stack, current_node, root);

    cleanup:
    free_node_stack(&stack);
    return result;

}

void _print_tree(Node* root, int depth){

    if (!root){
        return;
    }

    _print_tree(root->left, depth+1);

    for(int i = 0; i<depth; i++){
        printf("  ");
    }
    printf(VALUE_FORMAT, root->value);
    printf(" %d %d\n", root->blackness, root->black_height);

    _print_tree(root->right, depth+1);
}

void print_tree(Node* root){
    _print_tree(root, 0);
}

#ifdef TOPLEVEL
int main(int argc, char ** argv){

    Node* root = NULL;

    for(int i = 0; i<100; i++){
        find(i, &root, true);
        printf("--------------------------------------\n");
        print_tree(root);
    }

    printf("%d\n", sizeof(Node));

}
#endif
