
#define VALUE int
#define VALUE_FORMAT "%d\n"

struct node{
    VALUE value;
    struct node * left;
    struct node * right;
};

typedef struct node node;

// Include stacks
#define STACKTYPE node*
#define STACKNAME node_stack
#define PUSH node_push
#define POP node_pop
#define INIT init_node_stack

