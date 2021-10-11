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

VALUE splay(VALUE value, node** root, bool insert){
    
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

    while(currentnode && currentnode->value != value){
        node_push(&stack, currentnode);
        if (currentnode->value > value){
            currentnode = currentnode->left;
        } else {
            currentnode = currentnode->right;
        }
    }

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

    //TODO actual splaying

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




int main(int argc, char ** argv)
{
    node* root = NULL;

    for(int i = 0; i<10; i++){
        splay(i, &root, true);
    }
    print_tree(root);

}
