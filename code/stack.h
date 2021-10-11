typedef struct {
    STACKTYPE *array;
    size_t used;
    size_t size;
} STACKNAME;


void PUSH(STACKNAME* stack, STACKTYPE value);
STACKTYPE POP(STACKNAME* stack);
void INIT(STACKNAME *a, size_t initialSize);
