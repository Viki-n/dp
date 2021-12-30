#include <stdlib.h>
#include <stddef.h>


struct STACKNAME {
    STACKTYPE *array;
    size_t used;
    size_t size;
};
typedef struct STACKNAME STACKNAME;



void INIT(STACKNAME *a, size_t initialSize) {
    a->array = malloc(initialSize * sizeof(STACKTYPE));
    a->used = 0;
    a->size = initialSize;
}

void DESTROY(STACKNAME *a) {
    free(a->array);
}


void PUSH(STACKNAME* stack, STACKTYPE value){
    if (stack->used == stack->size) {
        stack->size *= 2;
        stack->array = realloc(stack->array, stack->size * sizeof(STACKTYPE));
    }
    stack->array[stack->used++] = value;
}


STACKTYPE POP(STACKNAME* stack){
    return stack->array[--(stack->used)];
}

#ifdef PEEK
STACKTYPE PEEK(STACKNAME* stack){
    return stack->array[stack->used - 1];
}
#undef PEEK
#endif

#undef STACKNAME
#undef STACKTYPE
#undef INIT
#undef DESTROY
#undef PUSH
#undef POP

