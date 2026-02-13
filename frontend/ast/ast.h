#ifndef AST_H
#define AST_H
typedef struct A_stm_ *A_stm;
typedef struct A_exp_ *A_exp;

typedef enum A_binop {A_add, A_sub, A_mul, A_div};

typedef struct {int startchar, line;} A_pos;


A_stm A_If_stm(A_pos pos, A_exp condition, A_stm thenbranch, A_stm elsebranch);
A_stm A_Ret_stm(A_exp returnval);
A_stm A_Expr_stm(A_exp exp);


struct A_stm_ {
    enum {A_if_stm, A_ret_stm, A_expr_stm} kind; 
    A_pos pos;
    union {
        struct {A_exp condition; A_stm thenbranch; A_stm elsebranch;} A_if_stm;
        struct {A_exp returnval;} A_ret_stm;
        struct {A_exp exp;} A_expr_stm;
    } u;
};

struct A_exp_ {
    enum {A_assign, A_binop, A_unary, A_cast} kind;
    A_pos pos;
    union {
        struct {/*identifier,*/ A_exp assignment;} A_assign;
        struct {A_exp left; enum A_binop op; A_exp right;} A_binop;
        struct {/*operator*/ A_exp operand;} A_unary;
        struct {/*cast type*/ A_exp operand;} A_cast;
    } u;
};

#endif