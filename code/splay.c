#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define VALUE int
#define VALUE_FORMAT "%d"

struct Node{
    VALUE value;
    struct Node * left;
    struct Node * right;
};

typedef struct Node Node;

// Include stack
#define STACKTYPE Node*
#define STACKNAME node_stack
#define PUSH node_push
#define POP node_pop
#define INIT init_node_stack
#define DESTROY free_node_stack

#include "stack.h"

void rotate_left(Node** n){

    Node* father = *n;
    Node* rson = father->right;
    Node* rlson = rson->left;
    father->right = rlson;
    rson->left = father;
    *n = rson;
} 


void rotate_right(Node ** n){

    Node* father = *n;
    Node* lson = father->left;
    Node* lrson = lson->right;
    father->left = lrson;
    lson->right = father;
    *n = lson;
}


void rotate_up(Node** parent, Node* son){

    if ((*parent)->right == son){
        rotate_left(parent);
    } else {
        rotate_right(parent);
    }
}


VALUE splay(VALUE value, Node** root, bool insert){
    // if insert, insert if not found (silently just splay if found either way)

    // Handle empty tree separately
    if (!*root){
        if (insert){
            Node* new = malloc(sizeof(Node));
            new->right = NULL;
            new->left = NULL;
            new->value = value;
            *root = new;
            return value;
        } else {
            return 0;
        }
    }

    node_stack stack;
    init_node_stack(&stack, 8);
    Node* current_node = *root;

    // find
    while(current_node && current_node->value != value){
        node_push(&stack, current_node);
        if (current_node->value > value){
            current_node = current_node->left;
        } else {
            current_node = current_node->right;
        }
    }

    // handle insertion
    if (!current_node){
        current_node = node_pop(&stack);
        if (insert){
            Node* new = malloc(sizeof(Node));
            new->right = NULL;
            new->left = NULL;
            new->value = value;
            if (current_node->value > value){
                current_node->left = new;
            } else {
                current_node->right = new;
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

            bool first_left = (current_node->left == son);
            bool second_left = (son->left == grandson);

            if (first_left == second_left){
                // Need to do zigzig rotation
                rotate_up(&current_node, son);
                rotate_up(&current_node, grandson);
            } else {
                // Perform zigzag rotation
                Node** first_target = first_left ? &current_node->left : &current_node->right;
                rotate_up(first_target, grandson);
                rotate_up(&current_node, grandson);
            }

            if (stack.used){
                Node* father = node_pop(&stack);
                node_push(&stack, father);
                if (father->left == original){
                    father->left = current_node;
                } else {
                    father->right = current_node;
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


}
