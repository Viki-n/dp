#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef MULTISPLAY
#define MULTISPLAY
#define TOPLEVEL
#endif

#ifndef VALUE
#define VALUE int
#define VALUE_FORMAT "%d"
#endif

#ifndef INVALID
#define INVALID -1
#endif

#define MIN(x,y) ((x)>(y)?(y):(x))
#define MAX(x,y) ((x)>(y)?(x):(y))

#define DEEPER(x,y) (((y) == NULL) || (((x) != NULL) && ((x)->depth > (y)->depth))?(x):(y))

#include "common.c"

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
            
            // fix parents pointer
            if (stack->used){
                Node* father = node_peek(stack);
                if (TOUCH(father)->left == original){
                    TOUCH(father)->left = current_node;
                } else {
                    TOUCH(father)->right = current_node;
                }
            }
        }

    }

    *root = current_node;
    return (*root)->value;

}


void find_by_depth(Node* root, node_stack* stack, int depth, bool directionisright){
    // if direction is right, returns rightmost element with depth < depth. Otherwisel, leftmost.
    Node* current_node = root;
    while (!is_external(current_node) || current_node == root){
        Node* first_son;
        Node* second_son;
        
        if (directionisright){
            first_son = TOUCH(current_node)->right;
            second_son = TOUCH(current_node)->left;
        } else {
            first_son = TOUCH(current_node)->left; 
            second_son = TOUCH(current_node)->right;
        }

        node_push(stack, current_node);
        if(!is_external(first_son) && TOUCH(first_son)->mindepth < depth){
            current_node = first_son;
        } else if (TOUCH(current_node)->depth < depth){ 
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
    int depth = rootpointer -> depth;

    if(is_external(TOUCH(rootpointer)->right)){
        if(TOUCH(rootpointer)->right){
            TOUCH(TOUCH(rootpointer)->right)->root = false;
        }
    } else if (TOUCH(TOUCH(rootpointer)->right)->mindepth > depth){
        TOUCH(TOUCH(rootpointer)->right)->root = true;
    } else {
        find_by_depth(TOUCH(rootpointer)->right, stack, TOUCH(rootpointer)->depth, false);
        splay(node_pop(stack), &(TOUCH(rootpointer)->right), stack);
        if(TOUCH(TOUCH(rootpointer)->right)->left){
            TOUCH(TOUCH(TOUCH(rootpointer)->right)->left)->root = !TOUCH(TOUCH(rootpointer)->right)->left->root;
            FIX(rootpointer->right);
        }
    }

    if(is_external(TOUCH(rootpointer)->left)){
        if(TOUCH(rootpointer)->left){
            TOUCH(TOUCH(rootpointer)->left)->root = false;
        }
    } else if (TOUCH(TOUCH(rootpointer)->left)->mindepth > depth){
        TOUCH(TOUCH(rootpointer)->left)->root = true;
    } else {
        find_by_depth(TOUCH(rootpointer)->left, stack, TOUCH(rootpointer)->depth, true);
        splay(node_pop(stack), &(TOUCH(rootpointer)->left), stack);
        if(TOUCH(TOUCH(rootpointer)->left)->right){
            TOUCH(TOUCH(TOUCH(rootpointer)->left)->right)->root = !TOUCH(TOUCH(rootpointer)->left)->right->root;
            FIX(TOUCH(rootpointer)->left);
        }
    }
    
    FIX(rootpointer);
    (**root).root = true;
}

VALUE find(VALUE value, Node** root){

    // Handle empty tree separately
    if (!*root){
        return INVALID;
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
    
    Node* right = NULL;
    Node* left = NULL;

    while(current_node){
        
        if(TOUCH(current_node)->root){
            stack = malloc(sizeof(node_stack));
            init_node_stack(stack, 8);
            stack_push(&metastack, stack);
            handle_push(&rootpointers, way);
            node_push(&parents, DEEPER(right, left));
            right = NULL;
            left = NULL;
        }

        node_push(stack, current_node);
        
        if (TOUCH(current_node)->value == value){
            break;
        }
        
        if (TOUCH(current_node)->value > value){
            way = &TOUCH(current_node)->left;
            right = current_node;
        } else {
            way = &TOUCH(current_node)->right;
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
    while (TOUCH(current_node)->value != value){
        //recycle parents stack for final switch
        node_push(&parents, current_node);
         if (TOUCH(current_node)->value > value){
            current_node = TOUCH(current_node)->left;
        } else {
            current_node = TOUCH(current_node)->right;
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
    Node* result = get_node();

#ifdef _TOUCH
    result->version = 0;
#endif    
    result->value = center;
    result->root = root;
    result->depth = depth;
    result->mindepth = depth;
    result->left = build_(lo, center-1, depth + 1, true);
    result->right = build_(center+1, hi, depth + 1, false);

    FIX(result);

    return result;
}

Node* build(int hi){
    init_memory(hi+1);
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

#ifdef TOPLEVEL
int main(int argc, char ** argv){

    Node* root = build(4);
    print_tree(root);
    for (int j =0; j<3; j++){
    for(int i = 0; i<=4; i++){
        find(i, &root);
        printf("--------------------------------------  %d\n", i);
        print_tree(root);
    }
    }
    printf("%d\n", sizeof(Node));

}
#endif
