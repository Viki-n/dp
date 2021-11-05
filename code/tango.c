#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define VALUE int
#define VALUE_FORMAT "%d"

#define MIN(x,y) ((x)>(y)?(y):(x))
#define MAX(x,y) ((x)>(y)?(x):(y))

struct Node{
    VALUE value;
    short blackness;    // during rebalance, it may make sense to have more than 2 possible values
    bool root;
    int black_height;
    int depth;
    int mindepth;
    int maxdepth;
    struct Node * left;
    struct Node * right;
};

bool is_external(Node* v){
    return v == NULL || v->root;
}

void fix_max_min_depth(Node* v){
    if(v == NULL){
        return;
    }
    int mindepth = v->depth;
    int maxdepth = v->depth;
    if(!is_external(v->left)){
        maxdepth = MAX(maxdepth, left->maxdepth);
        mindepth = MIN(mindepth, left->mindepth);
    }
    if(!is_external(v->right)){
        maxdepth = MAX(maxdepth, right->maxdepth);
        mindepth = MIN(mindepth, right->mindepth);
    }
}

typedef struct Node Node;

// Include stack
#define STACKTYPE Node*
#define STACKNAME node_stack
#define PUSH node_push
#define POP node_pop
#define INIT init_node_stack
#define DESTROY free_node_stack
#define PEEK node_peek

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

    Node* oldparent = *parent;

    if ((*parent)->right == son){
        rotate_left(parent);
    } else {
        rotate_right(parent);
    }

    fix_max_min_depth(oldparent);
    fix_max_min_depth(son);
}


Node* sibling(Node* v, Node* parent){
    if (v == parent->left){
        return parent->right;
    } else { 
        return parent->left;
    }
}

void push_blackness(Node* v){
    v->blackness--;
    v->left->blackness++;
    v->left->black_height++;
    v->right->blackness++;
    v->right->black_height++;
}

void suck_blackness(Node* v){
    v->blackness++;
    v->left->blackness--;
    v->left->black_height--;
    v->right->blackness--;
    v->right->black_height--;
}

void rebalance_after_insert(node_stack stack, Node* current_node, Node** root){
// Expects current_node pointing to a vertex that is red, both children black (or missing) and its parent may or may not be red too
// Expects that whole path between current_node and root is on the stack, including root, not including current_node
// Expects caller to handle freeing up stack
    while (stack.used){
        current_node = node_pop(&stack);
        if (current_node->blackness==1){
            return;
        }
        if (current_node == *root){
            current_node->blackness = 1;
            return;
        }
        Node* parent = current_node;
        current_node = node_pop(&stack);
        Node* uncle = sibling(parent,current_node);
        if (!is_external(uncle) && uncle->blackness == 0){
            push_blackness(current_node);
            continue;
        } else {
            Node** currentpointer = NULL;
            if(current_node == *root){
                currentpointer = root;
            } else {
                Node* grandgrandparent = node_peek(&stack);
                if (grandgrandparent->left == current_node){
                    currentpointer = &(grandgrandparent->left);
                } else {
                    currentpointer = &(grandgrandparent->right); 
                }
            }
            current_node->blackness = 0;
            current_node->black_height--;
            parent->blackness = 1;
            parent->black_height++;
            rotate_up(currentpointer, parent);
            return;
        }
    }
}

void join(Node** rootpointer){
    //Expects a node whose both subtrees are valid RB trees, but the tree as a whole doesn't have to be valid RB tree.
    //Root node blackness and black_height is ignored and thus can contain arbitrary values

    Node* root = *rootpointer;
    //handle case when nothing needs to be done
    int leftdepth = root->left==NULL? 0 : root->left->black_height;
    int rightdepth = root->left==NULL? 0 : root->right->black_height;
    if (rightdepth == leftdepth){
        root->blackness = 1;
        root->black_height = leftdepth +1;
        return;
    }
    Node* left = root->left;
    Node* right = root->right;

    //check if roots are black, and if not, restart procedure
    if (left != NULL && left->blackness == 0){
        left->blackness = 1;
        left->black_height++;
        join(rootpointer);
        return;
    }

    if (right != NULL && right->blackness == 0){
        right->blackness = 1;
        right->black_height++;
        join(rootpointer);
        return;
    }

    node_stack stack;
    init_node_stack(&stack, 8);
    root->blackness = 0;

    Node* current_node = NULL;

    if (rightdepth > leftdepth){

        *rootpointer = right;
        current_node = right;
        root->black_height = leftdepth;
    
        while(rightdepth > leftdepth){
            if(current_node->blackness){
                rightdepth--;
            }
            node_push(&stack, current_node);
            current_node = current_node->left;
        }

        root->right = current_node;
        Node* parent = node_peek(&stack);
        parent->left = root;
    
    } else {
 
        *rootpointer = left;
        current_node = left;
        root->black_height = rightdepth;
    
        while(rightdepth < leftdepth){
            if(current_node->blackness){
                leftdepth--;
            }
            node_push(&stack, current_node);
            current_node = current_node->right;
        }

        root->left = current_node;
        Node* parent = node_peek(&stack);
        parent->right = root;
    
    }

    node_push(&stack, root);
    if(current_node != NULL && current_node->blackness == 0){
        rebalance_after_insert(stack, current_node, rootpointer);
    }
    free_node_stack(&stack);
}

void split(Node** root, VALUE value){
// Will crash if value is not in tree. Returns a tree whose root has given value and both subtrees are valid rb trees. Blackness and black_height of root node can have an arbitrary value.
    
    Node* current_node = *root;
    node_stack leftstack;
    init_node_stack(&leftstack, 8);
    node_stack rightstack;
    init_node_stack(&rightstack, 8);
    
    while(current_node->value != value){
        if (current_node->value > value){
            node_push(&rightstack, current_node);
            current_node = current_node->left;
        } else {
            node_push(&leftstack, current_node);
            current_node = current_node->right;        
        }
    }
        
    Node* left = current_node->left;
     
    while (leftstack.used){
        Node* tmp = node_pop(&leftstack);
        tmp->right = left;
        left = tmp;
        join(&left);
    }

    Node* right = current_node->right;

    while (rightstack.used){
        Node* tmp = node_pop(&rightstack);
        tmp->left = right;
        right = tmp;
        join(&right);
    }

    current_node->left = left;
    current_node->right = right;
    *root = current_node;

    free_node_stack(&rightstack);
    free_node_stack(&leftstack);
}


VALUE find(VALUE value, Node** root){

    // Handle empty tree separately
    if (!*root){
        return 0;
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

    VALUE result = value;

    // handle insertion
    if (!current_node){
        current_node = node_pop(&stack);
        if (insert){
            Node* new = malloc(sizeof(Node));
            new->right = NULL;
            new->left = NULL;
            new->value = value;
            new->blackness = 0;
            new->black_height = 0;
            if (current_node->value > value){
                current_node->left = new;
            } else {
                current_node->right = new;
            }
            node_push(&stack, current_node);
            current_node = new;
        } else {
            result = current_node->value;
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

VALUE find_r(Node* root, int depth){
    int left_mindepth = depth+1;
    if(!is_external(root->left)){
        left_mindepth = root->left->mindepth;
    }
    int right_maxdepth = depth-1;
    if(!is_external(root->right)){
        right_maxdepth = root->right->maxdepth;
    }

    if(right_maxdepth >= depth){
        return find_r(root->right, depth);
    }
    if(left_mindepth < depth){
        result = find_r(root->left, depth);
        if (result == -1){
            return root->value;
        } else {
            return result;
        }
    }
    if(root->depth < depth){
        return root->value;
    } else {
        if (is_external(root->right)){
            return -1;
        } else {
            return find_r(root->right);
        }
    }
    
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


int main(int argc, char ** argv){

    Node* root = NULL;

    for(int i = 0; i<100; i++){
        find(i, &root, true);
        printf("--------------------------------------\n");
        print_tree(root);
    }


}
