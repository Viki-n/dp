#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef SPLAY
#define SPLAY
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

VALUE splay(VALUE value, Node** root, bool insert){
    // if insert, insert if not found (silently just splay if found either way)

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

    // handle insertion
    if (!current_node){
        current_node = node_pop(&stack);
        if (insert){
            Node* new = _allocated_memory ? get_node() : malloc(sizeof(Node));
#ifdef _TOUCH
            new->version = 0;
#endif
            TOUCH(new)->right = NULL;
            TOUCH(new)->left = NULL;
            TOUCH(new)->value = value;
            if (TOUCH(current_node)->value > value){
                TOUCH(current_node)->left = new;
            } else {
                TOUCH(current_node)->right = new;
            }
            node_push(&stack, current_node);
            current_node = new;
        }
    }

    // actual splay
    while (stack.used){
        if (stack.used == 1){
            Node* prev = current_node;
            current_node = node_pop(&stack);
            rotate_up(&current_node, prev);
        } else {
            Node* grandson = current_node;
            Node* son = node_pop(&stack);
            current_node = node_pop(&stack);
            Node* original = current_node;

            bool first_left = (TOUCH(current_node)->left == son);
            bool second_left = (TOUCH(son)->left == grandson);

            if (first_left == second_left){
                // Need to do zigzig rotation
                rotate_up(&current_node, son);
                rotate_up(&current_node, grandson);
            } else {
                // Perform zigzag rotation
                Node** first_target = first_left ? &TOUCH(current_node)->left : &TOUCH(current_node)->right;
                rotate_up(first_target, grandson);
                rotate_up(&current_node, grandson);
            }

            if (stack.used){
                Node* father = node_pop(&stack);
                node_push(&stack, father);
                if (TOUCH(father)->left == original){
                    TOUCH(father)->left = current_node;
                } else {
                    TOUCH(father)->right = current_node;
                }
            }
        }

    }

    *root = current_node;
    free_node_stack(&stack);
    return (*root)->value;

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
    printf("\n");

    _print_tree(root->right, depth+1);
}

void print_tree(Node* root){
    _print_tree(root, 0);
}

#ifdef TOPLEVEL
int main(int argc, char ** argv){

    Node* root = NULL;

    for(int i = 0; i<10; i++){
        splay(i, &root, true);
        printf("--------------------------------------\n");
        print_tree(root);
    }
    for(int i = 0; i<10; i++){
        splay(i, &root, true);
        printf("--------------------------------------\n");
        print_tree(root);
    }

    printf("%d\n", sizeof(Node));

}
#endif
