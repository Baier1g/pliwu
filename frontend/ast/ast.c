#include "ast.h"

A_stm A_If_stm(A_pos pos, A_exp condition, A_stm thenbranch, A_stm elsebranch){
    A_stm stm = malloc(sizeof stm);
    stm->kind = A_if_stm;
    stm->pos.line = pos.line;
    stm->pos.startchar = pos.startchar;
    stm->u.A_if_stm.condition = condition;
    stm->u.A_if_stm.thenbranch = thenbranch;
    stm->u.A_if_stm.elsebranch = elsebranch;
    return stm;
}

A_stm A_ret_stm(A_exp returnval){

}

A_stm A_expr_stm(A_exp exp){

}