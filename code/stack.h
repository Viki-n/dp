struct STACKNAME {
    STACKTYPE *array;
    size_t used;
    size_t size;
};

typedef struct STACKNAME STACKNAME;


void PUSH(STACKNAME* stack, STACKTYPE value);
STACKTYPE POP(STACKNAME* stack);
void INIT(STACKNAME *a, size_t initialSize);
