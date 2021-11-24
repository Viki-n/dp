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

struct Tuple {
    VALUE first;
    VALUE second;
};

typedef struct Node Node;
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
        maxdepth = MAX(maxdepth, v->left->maxdepth);
        mindepth = MIN(mindepth, v->left->mindepth);
    }
    if(!is_external(v->right)){
        maxdepth = MAX(maxdepth, v->right->maxdepth);
        mindepth = MIN(mindepth, v->right->mindepth);
    }
}

typedef struct Tuple Tuple;

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
        fix_max_min_depth(current_node);
        if (current_node->blackness==1){
            return;
        }
        if (current_node == *root){
            current_node->blackness = 1;
            return;
        }
        Node* parent = current_node;
        current_node = node_pop(&stack);
        fix_max_min_depth(current_node);
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
            fix_max_min_depth(current_node);
            fix_max_min_depth(parent);
            return;
        }
    }
}

void join(Node** rootpointer){
    //Expects a node whose both subtrees are valid RB trees, but the tree as a whole doesn't have to be valid RB tree.
    //Root node blackness and black_height is ignored and thus can contain arbitrary values

    Node* root = *rootpointer;
    //handle case when nothing needs to be done
    int leftdepth = is_external(root->left) ? 0 : root->left->black_height;
    int rightdepth = is_external(root->right) ? 0 : root->right->black_height;
    if (rightdepth == leftdepth){
        root->blackness = 1;
        root->black_height = leftdepth +1;
        return;
    }
    Node* left = root->left;
    Node* right = root->right;

    //check if roots are black, and if not, restart procedure
    if (!is_external(left) && left->blackness == 0){
        left->blackness = 1;
        left->black_height++;
        join(rootpointer);
        return;
    }

    if (!is_external(right) && right->blackness == 0){
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

    if(!is_external(current_node) && current_node->blackness == 0){
        rebalance_after_insert(stack, root, rootpointer);
    }
    while (stack.used){
        fix_max_min_depth(node_pop(&stack));
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
    if(is_external(root->left)){
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
            fix_max_min_depth(rootpointer);
            join(&rootpointer);
        } else if (l == -1){
            split(&rootpointer, r);
            rootpointer->left->root = true;
            fix_max_min_depth(rootpointer);
            join(&rootpointer);
        } else {
            split(&rootpointer, r);
            split(&(rootpointer->left), l);
            rootpointer->left->right->root = true;
            fix_max_min_depth(rootpointer->left);
            fix_max_min_depth(rootpointer);
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
        fix_max_min_depth(rootpointer);
        join(&rootpointer);
    } else if (l == -1){
        split(&rootpointer, r);
        rootpointer->left->root = false;
        fix_max_min_depth(rootpointer);
        join(&rootpointer);
    } else {
        split(&rootpointer, r);
        split(&(rootpointer->left), l);
        rootpointer->left->right->root = false;
        fix_max_min_depth(rootpointer->left);
        fix_max_min_depth(rootpointer);
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
        depth += 10;
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

    Node* root = build(100);
    print_tree(root);

    for(int i = 1; i<101; i++){
        find(i, &root);
        printf("--------------------------------------  %d\n", i);
        print_tree(root);
    }


}
