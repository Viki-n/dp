
void INIT(ARRAYNAME *a, size_t initialSize) {
    a->array = malloc(initialSize * sizeof(ARRAYTYPE));
    a->used = 0;
    a->size = initialSize;
}

void PUSH(STACKNAME stack, STACKTYPE value){
    if (stack->used == stack->size) {
        stack->size *= 2;
        stack->array = realloc(a->array, a->size * sizeof(ARRAYTYPE));
    }
    stack->array[stack->used++] = value;
}


STACKTYPE POP(STACKNAME* stack){
    return stack->array[--(stack->used)];
}
