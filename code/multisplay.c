#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define VALUE int
#define VALUE_FORMAT "%d"

#define MIN(x,y) ((x)>(y)?(y):(x))
#define MAX(x,y) ((x)>(y)?(x):(y))

#define DEEPER(x,y) (((y) == NULL) || (((x) != NULL) && (x->depth))>(y->depth)?(x):(y))

struct Node{
    VALUE value;
    bool root;
    int depth;
    int mindepth;
    struct Node * left;
    struct Node * right;
};

struct Tuple {
    VALUE first;
    VALUE second;
};

typedef struct Node Node;
bool is_external(Node* v){
    return v == NULL || v->root;
}

void fix_min_depth(Node* v){
    if(v == NULL){
        return;
    }
    int mindepth = v->depth;
    if(!is_external(v->left)){
        mindepth = MIN(mindepth, v->left->mindepth);
    }
    if(!is_external(v->right)){
        mindepth = MIN(mindepth, v->right->mindepth);
    }
    v->mindepth = mindepth;
}

typedef struct Tuple Tuple;

// Include stacks
#define STACKTYPE Node*
#define STACKNAME node_stack
#define PUSH node_push
#define POP node_pop
#define INIT init_node_stack
#define DESTROY free_node_stack
#define PEEK node_peek

#include "stack.h"

#define STACKTYPE Node**
#define STACKNAME handle_stack
#define PUSH handle_push
#define POP handle_pop
#define INIT init_handle_stack
#define DESTROY free_handle_stack

#include "stack.h"


#define STACKTYPE node_stack*
#define STACKNAME stack_stack
#define PUSH stack_push
#define POP stack_pop
#define INIT init_stack_stack
#define DESTROY free_stack_stack

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

    fix_min_depth(oldparent);
    fix_min_depth(son);
}


Node* sibling(Node* v, Node* parent){
    if (v == parent->left){
        return parent->right;
    } else { 
        return parent->left;
    }
}

VALUE splay(Node* current_node, Node** root, node_stack* stack){
    
    // actual splay
    while (stack->used){
        if (stack->used == 1){
            Node* prev = current_node;
            current_node = node_pop(stack);
            rotate_up(&current_node, prev);
        } else {
            Node* grandson = current_node;
            Node* son = node_pop(stack);
            current_node = node_pop(stack);
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
            
            // fix parents pointer
            if (stack->used){
                Node* father = node_peek(stack);
                if (father->left == original){
                    father->left = current_node;
                } else {
                    father->right = current_node;
                }
            }
        }

    }

    *root = current_node;
    return (*root)->value;

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

void find_by_depth(Node* root, node_stack* stack, int depth, bool directionisright){
    // if direction is right, returns rightmost element with depth < depth. Otherwisel, leftmost.
    Node* current_node = root;
    while (!is_external(current_node) || current_node == root){
        Node* first_son;
        Node* second_son;
        
        if (directionisright){
            first_son = current_node->right;
            second_son = current_node->left;
        } else {
            first_son = current_node->left; 
            second_son = current_node->right;
        }

        node_push(stack, current_node);
        if(!is_external(first_son) && first_son->mindepth < depth){
            current_node = first_son;
        } else if (current_node->depth < depth){ 
            return;
        } else {
            current_node = second_son;
        }
    }

}

void switch_direction(Node** root, Node* n, node_stack* stack){
    //assumes stack is filled by path from root to n including root excluding n. Returns empty stack.
    
    (**root).root = false;

    splay(n, root, stack);
    Node* rootpointer = *root;

    if(is_external(rootpointer->right)){
        if(rootpointer->right){
            rootpointer->right->root = false;
        }
    } else {
        find_by_depth(rootpointer->right, stack, rootpointer->depth, false);
        splay(node_pop(stack), &(rootpointer->right), stack);
        if(rootpointer->right->left){
            rootpointer->right->left->root ^= 1;
        }
    }

    if(is_external(rootpointer->left)){
        if(rootpointer->left){
            rootpointer->left->root = false;
        }
    } else {
        find_by_depth(rootpointer->left, stack, rootpointer->depth, true);
        splay(node_pop(stack), &(rootpointer->left), stack);
        if(rootpointer->left->right){
            rootpointer->left->right->root ^= true;
        }
    }

    (**root).root = true;
}

VALUE find(VALUE value, Node** root){

    // Handle empty tree separately
    if (!*root){
        return 0;
    }

    stack_stack metastack;
    init_stack_stack(&metastack, 8);

    handle_stack rootpointers;
    init_handle_stack(&rootpointers, 8);

    node_stack* stack = NULL;

    node_stack parents;
    init_node_stack(&parents, 8);

    Node** way = root;
    Node* current_node = *root;
    
    Node* right;
    Node* left;

    while(current_node){
        
        if(current_node->root){
            stack = malloc(sizeof(node_stack));
            init_node_stack(stack, 8);
            stack_push(&metastack, stack);
            handle_push(&rootpointers, way);
            node_push(&parents, DEEPER(right, left));
            right = NULL;
            left = NULL;
        }

        node_push(stack, current_node);
        
        if (current_node->value == value){
            break;
        }
        
        if (current_node->value > value){
            way = &current_node->left;
            right = current_node;
        } else {
            way = &current_node->right;
            left = current_node;
        }
        current_node = *way;

    }

    value = node_pop(stack)->value;

    free_node_stack(stack);
    free(stack);
    stack_pop(&metastack);
    handle_pop(&rootpointers);
    
    while(rootpointers.used){
    
        stack = stack_pop(&metastack);
        current_node = node_pop(stack);
        Node* parent = node_pop(&parents);
        Node** rootpointer = handle_pop(&rootpointers);

        while(current_node != parent){
            current_node = node_pop(stack);
        }

        switch_direction(rootpointer, parent, stack);

        free_node_stack(stack);
        free(stack);
    }

    node_pop(&parents);

    current_node = *root;
    while (current_node->value != value){
        //recycle parents stack for final switch
        node_push(&parents, current_node);
         if (current_node->value > value){
            current_node = current_node->left;
        } else {
            current_node = current_node->right;
        }
    }

    switch_direction(root, current_node, &parents);

    free_node_stack(&parents);
    free_handle_stack(&rootpointers);
    free_stack_stack(&metastack);

    return value;
}

Node* build_(int lo, int hi, int depth, bool root){
    int diff = hi-lo;
    if (diff < 0){
        return NULL;
    }
    int center = lo + diff/2;
    Node* result = malloc(sizeof(Node));
    
    result->value = center;
    result->root = root;
    result->depth = depth;
    result->mindepth = depth;
    result->left = build_(lo, center-1, depth + 1, true);
    result->right = build_(center+1, hi, depth + 1, false);

    fix_min_depth(result);

    return result;
}

Node* build(int hi){
    return build_(0, hi, 0, true);
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
    printf("%s %d %d\n", root->root ? " +": "", root->depth, root->mindepth);

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
