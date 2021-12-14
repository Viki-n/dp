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
#define FIND(x,y) find(x, y, false);
#endif
#else
#define FIND(x,y) find(x, y)
#endif

int main(int argc, char ** argv){
    int size;
    scanf("%d", &size);
    Node* tree = build(size);
#ifdef _TOUCH
    touches = 0;
    opId++;
#else
    clock_t t_start = clock(); 
#endif
    int query;
    while (scanf("%d", &query) == 1){
        FIND(query, &tree);
#ifdef _TOUCH
        opId++;
#endif
    }
#ifdef _TOUCH
    printf("Touches: %d\n", touches);
#else
    clock_t time = clock() - t_start;
    printf("Total time elapsed: %lf\n", (double)(time/(CLOCKS_PER_SEC / (double) 1000.0)));
#endif
}