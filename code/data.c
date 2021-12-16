#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

int rd(int max){
    if (INT_MAX <= RAND_MAX){
        return rand() % max;
    }
    int result = 0;
    int max_obtainable = 1;
    while (max > max_obtainable){
        result *= RAND_MAX + 1;
        result += rand();
        max_obtainable *= RAND_MAX + 1;
    }
    return result % max;
}

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
        printf("usage: %s tree_size sequence_length seed (s|r|u|b) [modifier]\n", argv[0]);
        printf("s  sequential\nr  random\nu  subset\nb  subset sequential\n");
        printf("u and b require an extra modifier -- size of subset\n");
        printf("\noutputs tree_size-1 on first line, and then sequence_length\nnumbers from [0,tree_size) generated using given mode.\n");
 
}

int main(int argc, char** argv) {
    if(argc<5) {
        help(argv);
        exit(1);
    }
    int size = atoi(argv[1]);
    int length = atoi(argv[2]);
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
