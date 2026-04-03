#ifndef IR_CODEGEN_H
#define IR_CODEGEN_H

typedef enum CG_op_kind CG_op_kind;
typedef struct CG_operation CG_operation;

enum CG_op_kind {
    MOV,
    LEA,
    PUSH,
    POP,

};

struct CG_operation {
    CG_op_kind operation;
    char *operand1;
    char *operand2;
};

#endif