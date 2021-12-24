#ifdef RANDOM_ALLOCATION
#include "random.c"
#endif

struct Node{
    VALUE value;
#if defined(REDBLACK) || defined(TANGO)
    short blackness;    // during rebalance, it may make sense to have more than 2 possible values
    short black_height;
#endif
#if defined(TANGO) || defined(MULTISPLAY)
    bool root;
    short depth;
    short mindepth;
#ifdef TANGO
    short maxdepth;
#endif
#endif
    struct Node * left;
    struct Node * right;
#ifdef TOUCH
    int version;
#endif
};

typedef struct Node Node;

#ifdef TOUCH

long long touches = 0;
int opId = 1;

static Node* touch(Node* n){
    if (n->version != opId){
        n->version = opId;
        touches++;
    }
    return n;
}
#undef TOUCH
#define TOUCH(x) touch(x)
#define _TOUCH
#else
#define TOUCH(x) (x)
#endif

static bool is_external(Node* v){
#if defined(TANGO) || defined(MULTISPLAY)
    return v == NULL || TOUCH(v)->root;
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
static void fix_depth(Node* v){
    if(v == NULL){
        return;
    }
    int mindepth = TOUCH(v)->depth;
    if(!is_external(TOUCH(v)->left)){
        mindepth = MIN(mindepth, TOUCH(TOUCH(v)->left)->mindepth);
    }
    if(!is_external(TOUCH(v)->right)){
        mindepth = MIN(mindepth, TOUCH(TOUCH(v)->right)->mindepth);
    }
    TOUCH(v)->mindepth = mindepth;
#ifdef TANGO
    int maxdepth = TOUCH(v)->depth;
    if(!is_external(TOUCH(v)->left)){
        maxdepth = MAX(maxdepth, TOUCH(TOUCH(v)->left)->maxdepth);
    }
    if(!is_external(TOUCH(v)->right)){
        maxdepth = MAX(maxdepth, TOUCH(TOUCH(v)->right)->maxdepth);
    }
    TOUCH(v)->maxdepth = maxdepth;
#endif
}
#define FIX(x) fix_depth(x)
#else
#define FIX(x)
#endif

static void rotate_left(Node** n){

    Node* father = *n;
    Node* rson = TOUCH(father)->right;
    Node* rlson = TOUCH(rson)->left;
    TOUCH(father)->right = rlson;
    TOUCH(rson)->left = father;
    *n = rson;
} 


void rotate_right(Node ** n){

    Node* father = *n;
    Node* lson = TOUCH(father)->left;
    Node* lrson = TOUCH(lson)->right;
    TOUCH(father)->left = lrson;
    TOUCH(lson)->right = father;
    *n = lson;
}


void rotate_up(Node** parent, Node* son){

    Node* oldparent = *parent;

    if (TOUCH(*parent)->right == son){
        rotate_left(parent);
    } else {
        rotate_right(parent);
    }

    FIX(oldparent);
    FIX(son);
}


Node* sibling(Node* v, Node* parent){
    if (v == TOUCH(parent)->left){
        return TOUCH(parent)->right;
    } else { 
        return TOUCH(parent)->left;
    }
}


#if defined(REDBLACK) || defined(TANGO)

void push_blackness(Node* v){
    TOUCH(v)->blackness--;
    TOUCH(v->left)->blackness++;
    v->left->black_height++;
    TOUCH(v->right)->blackness++;
    v->right->black_height++;
}

static void suck_blackness(Node* v){
    TOUCH(v)->blackness++;
    TOUCH(v->left)->blackness--;
    v->left->black_height--;
    TOUCH(v->right)->blackness--;
    v->right->black_height--;
}

static Node** get_self_pointer(Node* n, Node* p){
    if (TOUCH(p)->left == n){
        return &(TOUCH(p)->left);
    } else {
        return &(TOUCH(p)->right); 
    }
}

void rebalance_after_insert(node_stack stack, Node* current_node, Node** root){
// Expects current_node pointing to a vertex that is red, both children black (or missing) and its parent may or may not be red too
// Expects that whole path between current_node and root is on the stack, including root, not including current_node
// Expects caller to handle freeing up stack
    while (stack.used){
        Node* prev = current_node;
        current_node = node_pop(&stack);
        FIX(current_node);
        if (TOUCH(current_node)->blackness==1){
            return;
        }
        if (current_node == *root){
            TOUCH(current_node)->blackness = 1;
            current_node->black_height += 1;
            return;
        }
        Node* parent = current_node;
        current_node = node_pop(&stack);
        FIX(current_node);
        Node* uncle = sibling(parent,current_node);
        if (!is_external(uncle) && TOUCH(uncle)->blackness == 0){
            push_blackness(current_node);
            continue;
        } else {
            Node** currentpointer = NULL;
            if(current_node == *root){
                currentpointer = root;
            } else {
                Node* grandgrandparent = node_peek(&stack);
                if (TOUCH(grandgrandparent)->left == current_node){
                    currentpointer = &(TOUCH(grandgrandparent)->left);
                } else {
                    currentpointer = &(TOUCH(grandgrandparent)->right); 
                }
            }

            if((prev == parent->left) != (parent == current_node->left)){
                rotate_up(get_self_pointer(parent, current_node), prev);
                parent = prev;
            }

            TOUCH(current_node)->blackness = 0;
            TOUCH(current_node)->black_height--;
            TOUCH(parent)->blackness = 1;
            TOUCH(parent)->black_height++;
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
    int leftdepth = is_external(TOUCH(root)->left) ? 0 : TOUCH(TOUCH(root)->left)->black_height;
    int rightdepth = is_external(TOUCH(root)->right) ? 0 : TOUCH(TOUCH(root)->right)->black_height;
    if (rightdepth == leftdepth){
        TOUCH(root)->blackness = 1;
        TOUCH(root)->black_height = leftdepth +1;
        FIX(root);
        return;
    }
    Node* left = TOUCH(root)->left;
    Node* right = TOUCH(root)->right;

    //check if roots are black, and if not, restart procedure
    if (!is_external(left) && TOUCH(left)->blackness == 0){
        TOUCH(left)->blackness = 1;
        TOUCH(left)->black_height++;
        join(rootpointer);
        return;
    }

    if (!is_external(right) && TOUCH(right)->blackness == 0){
        TOUCH(right)->blackness = 1;
        TOUCH(right)->black_height++;
        join(rootpointer);
        return;
    }
#ifdef TANGO
    bool is_root_root = TOUCH(root)->root;
    TOUCH(root)->root = false;
#endif
    node_stack stack;
    init_node_stack(&stack, 8);
    TOUCH(root)->blackness = 0;

    Node* current_node = NULL;

    if (rightdepth > leftdepth){

        *rootpointer = right;
        current_node = right;
        TOUCH(root)->black_height = leftdepth;
        while(rightdepth > leftdepth){
            if(TOUCH(current_node)->blackness){
                rightdepth--;
            }
            node_push(&stack, current_node);
            current_node = TOUCH(current_node)->left;
        }

        TOUCH(root)->right = current_node;
        Node* parent = node_peek(&stack);
        TOUCH(parent)->left = root;
    
    } else {
 
        *rootpointer = left;
        current_node = left;
        TOUCH(root)->black_height = rightdepth;
    
        while(rightdepth < leftdepth){
            if(TOUCH(current_node)->blackness){
                leftdepth--;
            }
            node_push(&stack, current_node);
            current_node = TOUCH(current_node)->right;
        }

        TOUCH(root)->left = current_node;
        Node* parent = node_peek(&stack);
        TOUCH(parent)->right = root;
    
    }
    FIX(root);
    node_push(&stack, root);

#ifdef TANGO
    for (int i=0; i<stack.used; i++){
        FIX((stack.array)[i]);
    }
#endif
    if(!is_external(current_node) && TOUCH(current_node)->blackness == 0){
        rebalance_after_insert(stack, current_node, rootpointer);
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
    bool is_root_root = TOUCH(current_node)->root;
    TOUCH(current_node)->root = false;
#endif
    while(TOUCH(current_node)->value != value){
        if (TOUCH(current_node)->value > value){
            node_push(&rightstack, current_node);
            current_node = TOUCH(current_node)->left;
        } else {
            node_push(&leftstack, current_node);
            current_node = TOUCH(current_node)->right;        
        }
    }
        
    Node* left = TOUCH(current_node)->left;
     
    while (leftstack.used){
        Node* tmp = node_pop(&leftstack);
        TOUCH(tmp)->right = left;
        left = tmp;
        join(&left);
    }

    Node* right = TOUCH(current_node)->right;

    while (rightstack.used){
        Node* tmp = node_pop(&rightstack);
        TOUCH(tmp)->left = right;
        right = tmp;
        join(&right);
    }

    TOUCH(current_node)->left = left;
    TOUCH(current_node)->right = right;
    *root = current_node;
#ifdef TANGO
    TOUCH(current_node)->root = is_root_root;
#endif
    free_node_stack(&rightstack);
    free_node_stack(&leftstack);
}



#endif

#ifdef RANDOM_ALLOCATION
int* shuffled_array(int size){
    int* result = malloc(size*sizeof(int));
    for (int i=0; i<size; i++){
        result[i] = i;
    }
    for (int i=0; i<size; i++){
        int rd = next() % (size - i) + i;
        int tmp = result[i];
        result[i] = result[rd];
        result[rd] = tmp;
    }
    return result;
}
int* order = NULL;
#define SHUFFLE(x) order[x]
#else
#define SHUFFLE(x) x
#endif

Node* _allocated_memory = NULL;
int _allocated_memory_index = 0;
int _memory_alignment = 1 << 12;

void init_memory(int size){
    void* target = NULL;
    posix_memalign(&target, _memory_alignment, size*sizeof(Node));
    _allocated_memory = target;
#ifdef RANDOM_ALLOCATION
    for (int i=0; i<(size & 0x1ff); i++){
        jump();
    }
    order = shuffled_array(size);
#endif
}

Node* get_node(){

    return &(_allocated_memory[SHUFFLE(_allocated_memory_index++)]);
}

