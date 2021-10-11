#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "splay.h"
#include "stack.h"

void rotateLeft(node** Node){

    node* father = *Node;
    node* rson = father->right;
    node* rlson = rson->left;
    father->right = rlson;
    rson->left = father;
    *Node = rson;
} 


void rotateRight(node ** Node){

    node* father = *Node;
    node* lson = father->left;
    node* lrson = lson->right;
    father->left = lrson;
    lson->right = father;
    *Node = lson;
}


void rotate_up(node** parent, node* son){

    if ((*parent)->right == son){
        rotateLeft(parent);
    } else {
        rotateRight(parent);
    }
}


VALUE splay(VALUE value, node** root, bool insert){
    // if insert, insert if not found (silently just splay if found either way)

    // Handle empty tree separately
    if (!*root){
        if (insert){
            node* new = malloc(sizeof(node));
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
    node* currentnode = *root;

    // find
    while(currentnode && currentnode->value != value){
        node_push(&stack, currentnode);
        if (currentnode->value > value){
            currentnode = currentnode->left;
        } else {
            currentnode = currentnode->right;
        }
    }

    // handle insertion
    if (!currentnode){
        currentnode = node_pop(&stack);
        if (insert){
            node* new = malloc(sizeof(node));
            new->right = NULL;
            new->left = NULL;
            new->value = value;
            if (currentnode->value > value){
                currentnode->left = new;
            } else {
                currentnode->right = new;
            }
            node_push(&stack, currentnode);
            currentnode = new;
        }
    }

    // actual splay
    while (stack.used){
        if(stack.used == 1){
            node* prev = currentnode;
            currentnode = node_pop(&stack);
            rotate_up(&currentnode, prev);
        } else {
            node* grandson = currentnode;
            node* son = node_pop(&stack);
            currentnode = node_pop(&stack);
            node* original = currentnode;

            bool first_left = (currentnode->left == son);
            bool second_left = (son->left == grandson);

            if (first_left == second_left){
                // Need to do zigzig rotation
                rotate_up(&currentnode, son);
                rotate_up(&currentnode, grandson);
            } else {
                // Perform zigzag rotation
                node** first_target = first_left ? &currentnode->left : &currentnode->right;
                rotate_up(first_target, grandson);
                rotate_up(&currentnode, grandson);
            }

            if(stack.used){
                node* father = node_pop(&stack);
                node_push(&stack, father);
                if (father->left == original){
                    father->left = currentnode;
                } else {
                    father->right = currentnode;
                }
            }
        }

    }

    *root = currentnode;

    return (*root)->value;

}

void _print_tree(node* root, int depth){

    if(!root){
        return;
    }

    _print_tree(root->left, depth+1);

    for(int i = 0; i<depth; i++){
        printf("  ");
    }
    printf(VALUE_FORMAT, root->value);

    _print_tree(root->right, depth+1);
}

void print_tree(node* root){
    _print_tree(root, 0);
}


int main(int argc, char ** argv){

    node* root = NULL;

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
