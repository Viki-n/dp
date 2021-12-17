#ifdef TANGO 
#include "tango.c"
#endif

#ifdef MULTISPLAY
#include "multisplay.c"
#endif

#ifdef SPLAY
#include "splay.c"
#endif

#ifdef REDBLACK
#include "rb.c"
#endif

#ifndef _TOUCH
#include <time.h>
#endif

#if defined(REDBLACK) || defined(SPLAY)
//Splay and red-blask tree do not have build function, but they can insert on find
Node* build(int size){
    Node* tree = NULL;
    for(int i=0; i<=size; i++){
#ifdef SPLAY
        splay(i, &tree, true);
#else
        find(i, &tree, true);
#endif
    }
    return tree;
}
#ifdef SPLAY
#define FIND(x,y) splay(x, y, false);
#else
#define FIND(x,y) find(x, y, true);
#endif
#else
#define FIND(x,y) find(x, y)
#endif

int main(int argc, char ** argv){
    int* buffer = malloc(sizeof(int)*10000000);
    bool more_input = true;

    int size;
    scanf("%d", &size);
    Node* tree = build(size);
#ifdef _TOUCH
    touches = 0;
    opId++;
    
#else
    clock_t time = 0;  
#endif
    while (more_input){
        int buffer_len = 0;
        int buffer_capacity=10000000;
        while (buffer_len < buffer_capacity){
            int query;
            if(scanf("%d", &query) == 1){
                buffer[buffer_len] = query;
                buffer_len++;
            } else {
                more_input = false;
                break;
            }
        }
        #ifndef _TOUCH
            clock_t t_start = clock();
        #endif
        for (int i=0; i<buffer_len; i++){
            FIND(buffer[i], &tree);
#ifdef _TOUCH
            opId++;
#endif
        }
        #ifndef _TOUCH
            time += clock() - t_start;
        #endif
    }
#ifdef _TOUCH
    printf("Touches: %lli\n", touches);
#else
    printf("Total time elapsed: %lf ms\n", (double)(time/(CLOCKS_PER_SEC / (double) 1000.0)));
#endif
#if defined(TANGO) && defined(_TOUCH)
    check_tree(tree);
#endif
}