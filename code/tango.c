#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define TANGO

#define VALUE int
#define VALUE_FORMAT "%d"

#define MIN(x,y) ((x)>(y)?(y):(x))
#define MAX(x,y) ((x)>(y)?(x):(y))

#include "common.c"

struct Tuple {
    VALUE first;
    VALUE second;
};

typedef struct Tuple Tuple;



VALUE find_l(Node* root, int depth){
    
    // finds value of a element that has depth < than input, but its successor has depth >= than input. If such doesnt exist, return -1. 
    // Comments are switched!

    int left_maxdepth = depth-1;
    if(!is_external(root->left)){
        left_maxdepth = root->left->maxdepth;
    }

    // the target is either in right subtree or somewhere up the path to root
    if(left_maxdepth >= depth){
        return find_l(root->left, depth); 
    }
    
    // current node is the rightmost deep node
    if(root->depth >= depth){
        if(is_external(root->left)){
            // target is somewhere along the path upwards
            return -1;
        } else {
            // target is the minimum of right subtree
            return find_l(root->left, depth);
        }
    }

    // left subtree is missing, but I am not deep and anything to right isnt either -- I am target
    if(is_external(root->right)){
        return root->value;
    }
    
    // target is either me, or in the left subtree
    
    VALUE result = find_l(root->right, depth);
    return result == -1 ? root->value : result;
    
}



VALUE find_r(Node* root, int depth){
    
    // finds value of a element that has depth < than input, but its predcessor has depth >= than input. If such doesnt exist, return -1. 
    

    int right_maxdepth = depth-1;
    if(!is_external(root->right)){
        right_maxdepth = root->right->maxdepth;
    }

    // the target is either in right subtree or somewhere up the path to root
    if(right_maxdepth >= depth){
        return find_r(root->right, depth); 
    }
    
    // current node is the rightmost deep node
    if(root->depth >= depth){
        if(is_external(root->right)){
            // target is somewhere along the path upwards
            return -1;
        } else {
            // target is the minimum of right subtree
            return find_r(root->right, depth);
        }
    }

    // left subtree is missing, but I am not deep and anything to right isnt either -- I am target
    if(is_external(root->left)){
        return root->value;
    }
    
    // target is either me, or in the left subtree
    
    VALUE result = find_r(root->left, depth);
    return result == -1 ? root->value : result;
    
}

Tuple neighbors(Node* root, VALUE value){
    Tuple result;
    result.first = -1;
    result.second = -1;

    Node* current_node = root;
    while (current_node == root || !is_external(current_node)){
        if (current_node->value < value){
            result.first = current_node->value;
            current_node = current_node->right;
        } else {
            result.second = current_node->value;
            current_node = current_node->left; 
        }
    }

    return result;
}

void rebuild_current_subtree(Node** root, Node* new_part){

    Node* rootpointer = *root;
    
    VALUE r;
    VALUE l;
    
    // check if cutting is needed
    if(rootpointer->maxdepth >= new_part->maxdepth){
        r = find_r(rootpointer, new_part->mindepth);
        l = find_l(rootpointer, new_part->mindepth);

        if (r == -1){
            split(&rootpointer, l);
            rootpointer->right->root = true;
            FIX(rootpointer);
            join(&rootpointer);
        } else if (l == -1){
            split(&rootpointer, r);
            rootpointer->left->root = true;
            FIX(rootpointer);
            join(&rootpointer);
        } else {
            split(&rootpointer, r);
            split(&(rootpointer->left), l);
            rootpointer->left->right->root = true;
            FIX(rootpointer->left);
            FIX(rootpointer);
            join(&(rootpointer->left));
            join(&rootpointer);
        }
    }


    Tuple n = neighbors(rootpointer, new_part->value);
    l = n.first;
    r = n.second;

    if (r == -1){
        split(&rootpointer, l);
        rootpointer->right->root = false;
        FIX(rootpointer);
        join(&rootpointer);
    } else if (l == -1){
        split(&rootpointer, r);
        rootpointer->left->root = false;
        FIX(rootpointer);
        join(&rootpointer);
    } else {
        split(&rootpointer, r);
        split(&(rootpointer->left), l);
        rootpointer->left->right->root = false;
        FIX(rootpointer->left);
        FIX(rootpointer);
        join(&(rootpointer->left));
        join(&rootpointer);
    }
    
    *root = rootpointer;

}


VALUE find(VALUE value, Node** root){

    // Handle empty tree separately
    if (!*root){
        return 0;
    }

    Node* current_node = *root;
    Node* prev = *root;

    // find
    while(current_node && current_node->value != value){
        prev = current_node;
        if (current_node->value > value){
            current_node = current_node->left;
        } else {
            current_node = current_node->right;
        }
        
        if (current_node && current_node->root){
            rebuild_current_subtree(root, current_node);
        }
    }

    return current_node ? current_node->value : prev->value;
}

Node* build_(int lo, int hi, int depth){
    int diff = hi-lo;
    if (diff < 0){
        return NULL;
    }
    int center = lo + diff/2;
    Node* result = malloc(sizeof(Node));
    
    result->value = center;
    result->blackness = 1;
    result->root = true;
    result->black_height = 1;
    result->depth = depth;
    result->mindepth = depth;
    result->maxdepth = depth;
    result->left = build_(lo, center-1, depth + 1);
    result->right = build_(center+1, hi, depth + 1);

    return result;
}

Node* build(int hi){
    return build_(0, hi, 0);
}


void _print_tree(Node* root, int depth){

    if (!root){
        return;
    }

    if (root->root) {
        depth += 8;
    }

    _print_tree(root->left, depth+1);

    for(int i = 0; i<depth; i++){
        printf("  ");
    }
    printf(VALUE_FORMAT, root->value);
    printf(" %d %d%s %d %d %d\n", root->blackness, root->black_height, root->root ? " +": "", root->depth, root->mindepth, root->maxdepth);

    _print_tree(root->right, depth+1);
}

void print_tree(Node* root){
    _print_tree(root, -8);
}


int main(int argc, char ** argv){

    Node* root = build(126);
    print_tree(root);

    for(int i = 0; i<=126; i++){
        find(i, &root);
        printf("--------------------------------------  %d\n", i);
        print_tree(root);
    }


}
