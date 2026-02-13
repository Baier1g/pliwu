typedef enum {
    funcdecl,
    vardecl,
    assign,
    binop
} kind;

typedef struct supernode_ *supernode;

typedef struct supernode_ {
    enum {unary, binary, trinary} nodetype;
    int pos;
    int line;
    supernode next; //linked list for now ig
    union {
        struct {kind k; void *a}         unarynode;
        struct {kind k; void *a, *b}     binarynode;
        struct {kind k; void *a, *b, *c} trinarynode;
    } u;
};