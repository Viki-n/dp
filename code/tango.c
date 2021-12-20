#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef TANGO
#define TANGO
#define TOPLEVEL
#endif

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
    if(!is_external(TOUCH(root)->left)){
        left_maxdepth = TOUCH(TOUCH(root)->left)->maxdepth;
    }

    // the target is either in right subtree or somewhere up the path to root
    if(left_maxdepth >= depth){
        return find_l(TOUCH(root)->left, depth); 
    }
    
    // current node is the rightmost deep node
    if(TOUCH(root)->depth >= depth){
        if(is_external(TOUCH(root)->left)){
            // target is somewhere along the path upwards
            return -1;
        } else {
            // target is the minimum of right subtree
            return find_l(TOUCH(root)->left, depth);
        }
    }

    // left subtree is missing, but I am not deep and anything to right isnt either -- I am target
    if(is_external(TOUCH(root)->right)){
        return TOUCH(root)->value;
    }
    
    // target is either me, or in the left subtree
    
    VALUE result = find_l(TOUCH(root)->right, depth);
    return result == -1 ? TOUCH(root)->value : result;
    
}



VALUE find_r(Node* root, int depth){
    
    // finds value of a element that has depth < than input, but its predcessor has depth >= than input. If such doesnt exist, return -1. 
    

    int right_maxdepth = depth-1;
    if(!is_external(TOUCH(root)->right)){
        right_maxdepth = TOUCH(TOUCH(root)->right)->maxdepth;
    }

    // the target is either in right subtree or somewhere up the path to root
    if(right_maxdepth >= depth){
        return find_r(TOUCH(root)->right, depth); 
    }
    
    // current node is the rightmost deep node
    if(TOUCH(root)->depth >= depth){
        if(is_external(TOUCH(root)->right)){
            // target is somewhere along the path upwards
            return -1;
        } else {
            // target is the minimum of right subtree
            return find_r(TOUCH(root)->right, depth);
        }
    }

    // left subtree is missing, but I am not deep and anything to right isnt either -- I am target
    if(is_external(TOUCH(root)->left)){
        return TOUCH(root)->value;
    }
    
    // target is either me, or in the left subtree
    
    VALUE result = find_r(TOUCH(root)->left, depth);
    return result == -1 ? TOUCH(root)->value : result;
    
}

Tuple neighbors(Node* root, VALUE value){
    Tuple result;
    result.first = -1;
    result.second = -1;

    Node* current_node = root;
    while (current_node == root || !is_external(current_node)){
        if (TOUCH(current_node)->value < value){
            result.first = TOUCH(current_node)->value;
            current_node = TOUCH(current_node)->right;
        } else {
            result.second = TOUCH(current_node)->value;
            current_node = TOUCH(current_node)->left; 
        }
    }

    return result;
}

static void rebuild_current_subtree(Node** root, Node* new_part){

    Node* rootpointer = *root;


    VALUE r;
    VALUE l;
    
    // check if cutting is needed
    if(TOUCH(rootpointer)->maxdepth >= TOUCH(new_part)->mindepth){
        r = find_r(rootpointer, TOUCH(new_part)->mindepth);
        l = find_l(rootpointer, TOUCH(new_part)->mindepth);


        if (r == -1){
            split(&rootpointer, l);
            TOUCH(TOUCH(rootpointer)->right)->root = true;
            FIX(rootpointer);
            join(&rootpointer);
        } else if (l == -1){
            split(&rootpointer, r);
            TOUCH(TOUCH(rootpointer)->left)->root = true;
            FIX(rootpointer);
            join(&rootpointer);
        } else {
            split(&rootpointer, r);
            split(&(TOUCH(rootpointer)->left), l);
            TOUCH(TOUCH(TOUCH(rootpointer)->left)->right)->root = true;
            FIX(TOUCH(rootpointer)->left);
            FIX(rootpointer);
            join(&(TOUCH(rootpointer)->left));
            join(&rootpointer);
        }
    }
    Tuple n = neighbors(rootpointer, TOUCH(new_part)->value);
    l = n.first;
    r = n.second;

    if (r == -1){
        split(&rootpointer, l);
        TOUCH(TOUCH(rootpointer)->right)->root = false;
        FIX(rootpointer);
        join(&rootpointer);
    } else if (l == -1){
        split(&rootpointer, r);
        TOUCH(TOUCH(rootpointer)->left)->root = false;
        FIX(rootpointer);
        join(&rootpointer);
    } else {
        split(&rootpointer, r);
        split(&(TOUCH(rootpointer)->left), l);
        TOUCH(TOUCH(TOUCH(rootpointer)->left)->right)->root = false;
        FIX(TOUCH(rootpointer)->left);
        FIX(rootpointer);
        join(&(TOUCH(rootpointer)->left));
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
    while(current_node && TOUCH(current_node)->value != value){
        prev = current_node;
        if (TOUCH(current_node)->value > value){
            current_node = TOUCH(current_node)->left;
        } else {
            current_node = TOUCH(current_node)->right;
        }
        
        if (current_node && TOUCH(current_node)->root){
            rebuild_current_subtree(root, current_node);
            current_node = *root;
        }
    }

    return current_node ? TOUCH(current_node)->value : TOUCH(prev)->value;
}

Node* build_(int lo, int hi, int depth){
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
    init_memory(hi+1);
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


void push_tree(node_stack* s, Node* r, bool is_top){
    if(!is_top && is_external(r)){
        return;
    }
    node_push(s, r);
    push_tree(s,r->left,false);
    push_tree(s,r->right,false);
}

bool check_tree(Node* v){
    if (v == NULL){
        return true;
    }
        int mindepth = (v)->depth;
    if(!is_external((v)->left)){
        mindepth = MIN(mindepth, ((v)->left)->mindepth);
    }
    if(!is_external((v)->right)){
        mindepth = MIN(mindepth, ((v)->right)->mindepth);
    }
    if (v->mindepth != mindepth){
        printf("Wrong mindepth at %d!\n", v->value);
        return false;
    }
    int maxdepth = (v)->depth;
    if(!is_external((v)->left)){
        maxdepth = MAX(maxdepth, ((v)->left)->maxdepth);
    }
    if(!is_external((v)->right)){
        maxdepth = MAX(maxdepth, ((v)->right)->maxdepth);
    }
    if (v->maxdepth != maxdepth){
        printf("Wrong maxdepth at %d!\n", v->value);
        return false;
    }
    if (v->root){
        node_stack stack;
        init_node_stack(&stack, 8);
        push_tree(&stack, v, true);
        for (int i=0; i<stack.used; i++){
            for (int j=i+1; j<stack.used; j++){
                if(stack.array[i]->depth == stack.array[j]->depth){
                    printf("Wrong division in %d!\n", v->value);
                    return false;
                }
            }
        }
    }
    if(v->blackness == 0){
        if(!is_external(v->left)&& v->left->blackness==0){
            printf("Wrong color in %d!\n", v->value);
            return false; 
        }
        if(!is_external(v->right)&& v->right->blackness==0){
            printf("Wrong color in %d!\n", v->value);
            return false; 
        }
        if((is_external(v->left) || is_external(v->right)) && v->black_height!=0){
            printf("Wrong color in %d!\n", v->value);
            return false; 
        }
    }

    return check_tree(v->left) && check_tree(v->right);
}


#ifdef TOPLEVEL
int main(int argc, char ** argv){

    Node* root = build(126);
    print_tree(root);

    for(int i = 0; i<=126; i++){
        find(i, &root);
        printf("--------------------------------------  %d\n", i);
        print_tree(root);
    }
    printf("%d\n", sizeof(Node));


}
#endif
