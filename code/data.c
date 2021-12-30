#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "random.c"

int* subset(int max,int size){
    int* result = malloc(sizeof(int)*size);
    for (int i=0; i<size; i++){
        result[i] = rd(max);
    }
    return result;
}

int comp (const void * elem1, const void * elem2) 
{
    int f = *((int*)elem1);
    int s = *((int*)elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}

void help(char** argv){
        printf("\nInsufficient amount of arguments!\n\n");
        printf("usage: %s tree_size sequence_length seed (s|r|u|b|i) [modifier]\n", argv[0]);
        printf("s  sequential\nr  random\nu  subset\nb  subset sequential\ni  left spine bit reversal\n");
        printf("u and b require an extra modifier -- size of subset\n");
        printf("\noutputs tree_size-1 on first line, and then sequence_length\nnumbers from [0,tree_size) generated using given mode.\n");
 
}

int* reversal(){
    int* result = malloc(256*sizeof(int));
    for (int i=0; i<256; i++){
        int r = 0; 
        for(int b=0; b<8; b++){
            int t = i & (1 << b);
            t <<= 7;
            t >>= b*2;
            r |= t;
        }
        result[i] = r;
    }
    return result;
}

int* left_spine(int hi){
    int* result = malloc(256*sizeof(int));
    int index = 1;
    int lo = 0;
    result[0] = 0;
    for (;;){
        int diff = hi-lo;
        if (diff < 0){
            return result;
        }
        int center = lo + diff/2;
        result[index++] = center;
        hi = center-1;
        result[0]++;
    }
}

int main(int argc, char** argv) {
    if(argc<5) {
        help(argv);
        exit(1);
    }
    int size = atoi(argv[1]);
    int length = atoi(argv[2]);

    // we will not need to use many different seeds, but more than one might be nice
    for (int i=0; i<atoi(argv[3]); i++){
        jump();
    }

    srand(atoi(argv[3]));
    char mode = argv[4][0];
    printf("%d\n", size - 1);
    if (mode == 's'){
        for (int i=0; i<length; i++){
            printf("%d\n", i % (size));
        }
    } else if (mode == 'r'){
        for (int i=0; i<length; i++){
            printf("%d\n", rd(size));
        }
    } else if (mode == 'i'){
        int* spine = left_spine(size-1);
        int spine_size = spine[0];
        int* seq = reversal();
        int index = 0;
        for (int i = 0; i<length; index++){
            int target = seq[index & 0xff];
            if (target<spine_size){
                printf("%d\n", spine[target+1]);
                i++;
            }
        }
    } else {
        if(argc==5) {
            help(argv);
            exit(1);
        }
        int subset_size = atoi(argv[5]);
        int* set = subset(size, subset_size);
        if (mode == 'u'){
            for (int i=0; i<length; i++){
                printf("%d\n", set[rd(subset_size)]);
            }
        } else {
            qsort(set, subset_size, sizeof(int), comp);
            for (int i=0; i<length; i++){
                printf("%d\n", set[i%subset_size]);
            }

        }
    }

}
