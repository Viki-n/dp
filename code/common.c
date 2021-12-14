
struct Node{
    VALUE value;
#if defined(REDBLACK) || defined(TANGO)
    short blackness;    // during rebalance, it may make sense to have more than 2 possible values
    int black_height;
#endif
#if defined(TANGO) || defined(MULTISPLAY)
    bool root;
    int depth;
    int mindepth;
#ifdef TANGO
    int maxdepth;
#endif
#endif
    struct Node * left;
    struct Node * right;
};

typedef struct Node Node;

bool is_external(Node* v){
#if defined(TANGO) || defined(MULTISPLAY)
    return v == NULL || v->root;
#else
    return v == NULL;
#endif
}

// Include stack
#define STACKTYPE Node*
#define STACKNAME node_stack
#define PUSH node_push
#define POP node_pop
#define INIT init_node_stack
#define DESTROY free_node_stack
#define PEEK node_peek

#include "stack.h"

#ifdef MULTISPLAY
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
#endif

#if defined(TANGO) || defined(MULTISPLAY)
void fix_depth(Node* v){
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
#ifdef TANGO
    int maxdepth = v->depth;
    if(!is_external(v->left)){
        maxdepth = MAX(maxdepth, v->left->maxdepth);
    }
    if(!is_external(v->right)){
        maxdepth = MAX(maxdepth, v->right->maxdepth);
    }
    v->maxdepth = maxdepth;
#endif
}
#define FIX(x) fix_depth(x)
#else
#define FIX(x)
#endif

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

    FIX(oldparent);
    FIX(son);
}


Node* sibling(Node* v, Node* parent){
    if (v == parent->left){
        return parent->right;
    } else { 
        return parent->left;
    }
}


#if defined(REDBLACK) || defined(TANGO)

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
        FIX(current_node);
        if (current_node->blackness==1){
            return;
        }
        if (current_node == *root){
            current_node->blackness = 1;
            return;
        }
        Node* parent = current_node;
        current_node = node_pop(&stack);
        FIX(current_node);
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
            FIX(current_node);
            FIX(parent);
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
#ifdef TANGO
    bool is_root_root = root->root;
    root->root = false;
#endif
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

    if(!is_external(current_node) && current_node->blackness == 0){
        rebalance_after_insert(stack, root, rootpointer);
    }
#ifdef TANGO
    while (stack.used){
        FIX(node_pop(&stack));
    }
    (**rootpointer).root = is_root_root;
#endif
    free_node_stack(&stack);
}

void split(Node** root, VALUE value){
// Will crash if value is not in tree. Returns a tree whose root has given value and both subtrees are valid rb trees. Blackness and black_height of root node can have an arbitrary value.
    
    Node* current_node = *root;
    node_stack leftstack;
    init_node_stack(&leftstack, 8);
    node_stack rightstack;
    init_node_stack(&rightstack, 8);
#ifdef TANGO
    bool is_root_root = current_node->root;
    current_node->root = false;
#endif
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
#ifdef TANGO
    current_node->root = is_root_root;
#endif
    free_node_stack(&rightstack);
    free_node_stack(&leftstack);
}



#endif


