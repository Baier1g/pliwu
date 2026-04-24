#include "ir.h"

segment *current_segment;
frame *current_frame;
symbol_table *current_frame_table;
hash_map *local_variables;
int temp_counter = 1;
int if_counter = 0;
int while_counter = 0;


char *IR_op_code_to_string(IR_op_code op) {
    switch (op) {
        case IR_ASSIGN:
            return "assign";
        case IR_POP_PARAM:
            return "pop_param";
        case IR_PARAM:
            return "param";
        case IR_ADD:
            return "add";
        case IR_SUB:
            return "sub";
        case IR_MUL:
            return "mul";
        case IR_DIV:
            return "div";
        case IR_EQUALS:
            return "equals";
        case IR_NEQUALS:
            return "not_equals";
        case IR_LESS:
            return "less";
        case IR_GREATER:
            return "greater";
        case IR_LESS_EQ:
            return "less_equals";
        case IR_GREATER_EQ:
            return "greater_equals";
        case IR_AND:
            return "and";
        case IR_OR:
            return "or";
        case IR_IF:
            return "if";
        case IR_WHILE:
            return "while";
        case IR_PRINT:
            return "print";
        case IR_CALL:
            return "call";
        case IR_RET:
            return "return";
        case IR_GOTO:
            return "goto";
        case IR_VAR_DECL:
            return "var_decl";
        default:
            return "ir.c::IR_op_code_to_string: Unknown op_code";
    }
}

enum IR_op_code AST_op_to_IR_op(binary_op op) {
    switch(op) {
        case A_LESS:
            return IR_LESS;
        case A_GREATER:
            return IR_GREATER;
        case A_LESS_EQ:
            return IR_LESS_EQ;
        case A_GREATER_EQ:
            return IR_GREATER_EQ;
        case A_EQUALS:
            return IR_EQUALS;
        case A_NEQUALS:
            return IR_NEQUALS;
        case A_ADD:
            return IR_ADD;
        case A_SUB:
            return IR_SUB;
        case A_MULT:
            return IR_MUL;
        case A_DIV:
            return IR_DIV;
        case A_AND:
            return IR_AND;
        case A_OR:
            return IR_OR;
        default:
            printf("ir.c::AST_op_to_IR_op: Unknown binary_op\n");
            exit(-1);
    }
}

char *IR_generate_label(char* string, int i) {
    char *tmp = calloc(strlen(string) + 5, sizeof(char));
    sprintf(tmp, "%s%d", string, i);
    return tmp;
}

IR_operation *create_op(IR_op_code op, IR_operand *arg1, IR_operand *arg2, IR_operand *arg3) {
    IR_operation *oper = malloc(sizeof(IR_operation));
    oper->in = linked_list_new();
    oper->out = linked_list_new();
    oper->use = linked_list_new();
    oper->def = linked_list_new();
    oper->op = op;
    oper->arg1 = arg1;
    oper->arg2 = arg2;
    oper->arg3 = arg3;
    handle_use_set(oper);
    handle_def_set(oper);
    return oper;
}

frame *create_frame() {
    frame *tmp = (frame *) malloc(sizeof(frame));
    tmp->name = NULL;
    tmp->nested_frames = linked_list_new();
    tmp->max_offset = 0;
    tmp->func_params = 0;
    tmp->regs_used = 0;
    return tmp;
}

frame *create_named_frame(char *name) {
    frame *tmp = (frame *) malloc(sizeof(frame));
    tmp->name = calloc(strlen(name), sizeof(char));
    strcpy(tmp->name, name);
    tmp->nested_frames = linked_list_new();
    return tmp;
}

segment *create_segment(symbol_table *table) {
    segment *tmp = (segment *) malloc(sizeof(segment));
    tmp->left = tmp->right = NULL;
    tmp->iteration = 0;
    tmp->table = table;
    tmp->pred = linked_list_new();
    tmp->operations = linked_list_new();
    return tmp;
}

IR_operand *create_operand(operand_type type, void *content) {
    IR_operand *op = malloc(sizeof(IR_operand));
    op->type = type;
    switch(type) {
        case P_VARIABLE:
            op->variable_name = calloc(strlen((char *) content), sizeof(char));
            strcpy(op->variable_name, content);
            break;
        case P_TEMP:
        case P_CONSTANT:
            op->constant = (int) content;
            break;
        case P_LABEL:
            op->dest = (segment *) content;
            break;
        case P_FUNC_CALL:
            op->call = (frame *) content;
            break;
    }
    return op;
}

int recurse_IR_tree(AST_node *node) {
    if (!node) {
        return 0;
    }

    //printf("Current node kind is %s\n", kind_enum_to_string(node->kind));

    int condition;
    char *name;
    symbol_table *sym, *old_table;
    hash_map *tmp;
    frame *frm, *old_frame;
    segment *seg, *old_segment;
    IR_operation *op;
    IR_operand *expr, *id;
    switch (node->kind) {
        case A_MODULE:
            frm = create_frame();
            frm->segment = create_segment(node->table);
            linked_list_append(current_frame->nested_frames, frm);
            sym = create_symbol_table(current_frame_table, current_frame_table->global);
            old_table = current_frame_table;
            current_frame_table = sym;
            old_frame = current_frame;
            old_segment = current_segment;
            current_frame = frm;
            current_segment = frm->segment;
            tmp = local_variables;
            local_variables = create_hash_map(128);
            for (linked_list_node *lln = node->module.module_declarations->head; lln != NULL; lln = lln->next) {
                AST_node *n;
                //printf("Made it to %s in module\n", kind_enum_to_string(((AST_node *)lln->data)->kind));
                if (((AST_node *) lln->data)->kind == A_FUNC_DEF) {
                    n = (AST_node *) lln->data;
                    char *name = n->func_def.identifier->primary_expr.identifier_name;
                    //printf("got name\n");
                    frame *fr = create_named_frame(name);
                    //printf("Created frame\n");
                    linked_list_append(current_frame->nested_frames, fr);
                    //printf("appended frame\n");
                    symbol_table_insert(current_frame_table, name, fr);
                }
            }
            for (linked_list_node *lln = node->module.module_declarations->head; lln != NULL; lln = lln->next) {
                recurse_IR_tree((AST_node *) lln->data);
            }
            frm->locals = local_variables;
            local_variables = tmp;
            current_frame->last = current_segment;
            current_frame_table = old_table;
            current_frame = old_frame;
            current_segment = old_segment;
            break;
        case A_FUNC_DEF:
            frm = symbol_table_get(current_frame_table, node->func_def.identifier->primary_expr.identifier_name);
            frm->segment = create_segment(node->func_def.function_block->table);
            sym = create_symbol_table(current_frame_table, current_frame_table->global);
            old_table = current_frame_table;
            current_frame_table = sym;
            old_frame = current_frame;
            old_segment = current_segment;
            current_frame = frm;
            current_segment = frm->segment;
            for (linked_list_node *lln = node->func_def.function_block->block.stmt_list->head; lln != NULL; lln = lln->next) {
                AST_node *n;
                if ((n = ((AST_node *) lln->data))->kind == A_FUNC_DEF) {
                    char *name = n->func_def.identifier->primary_expr.identifier_name;
                    frame *fr = create_named_frame(name);
                    linked_list_append(current_frame->nested_frames, fr);
                    symbol_table_insert(current_frame_table, name, fr);
                }
            }
            
            tmp = local_variables;

            
            local_variables = create_hash_map(1024);
            frm->locals = local_variables;
            
            // Add function parameters to local variables
            int param_counter = 0;
            for (linked_list_node *lln = node->func_def.parameters->head; lln != NULL; lln = lln->next) {
                name = ((AST_node *) lln->data)->parameter.identifier->primary_expr.identifier_name;
                if (param_counter >= 4) {
                    var_info *info = (var_info *) symbol_table_get(node->func_def.function_block->table, name);
                    info->offset = -((param_counter - 1) * 8);
                }
                expr = create_operand(P_TEMP, temp_counter++);
                id = create_operand(P_VARIABLE, name);
                op = create_op(IR_VAR_DECL, expr, id, NULL);
                op->in_frame = current_frame;
                op->in_seg = current_segment;
                hash_map_insert(local_variables, name, expr);
                linked_list_append(current_segment->operations, op);
                param_counter++;
            }
            frm->func_params = param_counter;

            // Create segments of frame
            recurse_IR_tree(node->func_def.function_block);
            local_variables = tmp;

            current_frame->last = current_segment;
            current_frame_table = old_table;
            current_frame = old_frame;
            current_segment = old_segment;
            break;
        case A_VAR_DECL:
            name = node->var_decl.identifier->primary_expr.identifier_name;
            expr = NULL;
            if (node->var_decl.expr_stmt) {
                //int old_count = temp_counter;
                int new_count = recurse_IR_tree(node->var_decl.expr_stmt);
                //printf("%s\n", name);
                var_info *info = (var_info *) symbol_table_get(node->table, name);
                if (!info) {
                    printf("Not there :(\n");
                }
                if (((var_info *) symbol_table_get(current_segment->table, name))->escaping) {
                    id = create_operand(P_VARIABLE, name);
                } else {
                    id = create_operand(P_TEMP, temp_counter);
                }
                expr = create_operand(P_TEMP, new_count);
                temp_counter++;
                op = create_op(IR_VAR_DECL, id, expr, NULL);
                hash_map_insert(local_variables, name, id);
                linked_list_append(current_segment->operations, op);
            } else {
                id = create_operand(P_VARIABLE, name);
                expr = create_operand(P_CONSTANT, 0);
                op = create_op(IR_VAR_DECL, id, expr, NULL);
                op->in_frame = current_frame;
                op->in_seg = current_segment;
                linked_list_append(current_segment->operations, op);
            }
            //print_operation(op);
            break;
        case A_BLOCK_STMT:
            for (linked_list_node *lln = node->block.stmt_list->head; lln != NULL; lln = lln->next) {
                recurse_IR_tree((AST_node *) lln->data);
            }
            break;
        case A_IF_STMT:
            int if_c = if_counter++;
            condition = recurse_IR_tree(node->if_stmt.condition);
            current_segment->left = create_segment(node->if_stmt.if_branch->table);
            linked_list_append(current_segment->left->pred, current_segment);
            //printf("We alive\n");
            if (current_segment->right) {
                current_segment->right = create_segment(node->if_stmt.else_branch->table);
                current_segment->right->name = IR_generate_label("else", if_c);
                linked_list_append(current_segment->right->pred, current_segment);
            }
            seg = create_segment(node->table);
            seg->name = IR_generate_label("end_if", if_c);
            
            IR_operand *if_branch = create_operand(P_LABEL, current_segment->left);
            IR_operand *else_branch;
            if (current_segment->right) {
                else_branch = create_operand(P_LABEL, current_segment->right);
            } else {
                else_branch = create_operand(P_LABEL, seg);
            }
            op = create_op(IR_IF, create_operand(P_TEMP, condition), if_branch, else_branch);
            op->in_frame = current_frame;
            op->in_seg = current_segment;

            linked_list_append(current_segment->operations, op);
            linked_list_append(seg->pred, current_segment->left);
            if (current_segment->right) {
                linked_list_append(seg->pred, current_segment->right);
            }
            old_segment = current_segment;

            current_segment = current_segment->left;
            recurse_IR_tree(node->if_stmt.if_branch);
            current_segment->left = seg;
            if (current_segment->right) {
                op = create_op(IR_GOTO, create_operand(P_LABEL, seg), NULL, NULL);
                op->in_frame = current_frame;
                op->in_seg = current_segment;
                linked_list_append(current_segment->operations, op);

                current_segment = old_segment->right;
                recurse_IR_tree(node->if_stmt.else_branch);
                current_segment->left = seg;
            }
            current_segment = seg;
            break;
        case A_WHILE_LOOP:
            int while_c = while_counter++;
            seg = create_segment(node->table);
            seg->name = IR_generate_label("start_while", while_c);
            current_segment->left = seg;
            seg->left = create_segment(node->while_loop.block->table);
            linked_list_append(seg->pred, current_segment);
            current_segment = seg;
            //linked_list_append(current_segment->pred, seg);
            //current_segment->left->left = current_segment;
            current_segment->right = create_segment(node->table);
            current_segment->right->name = IR_generate_label("end_while", while_c);
            linked_list_append(current_segment->right->pred, current_segment);

            condition = recurse_IR_tree(node->while_loop.condition);
            IR_operand *arg2 = create_operand(P_LABEL, current_segment->left);
            IR_operand *arg3 = create_operand(P_LABEL, current_segment->right);
            op = create_op(IR_WHILE, create_operand(P_TEMP, condition), arg2, arg3);
            op->in_frame = current_frame;
            op->in_seg = current_segment;
            linked_list_append(current_segment->operations, op);
            
            current_segment = current_segment->left;
            recurse_IR_tree(node->while_loop.block);
            op = create_op(IR_GOTO, create_operand(P_LABEL, seg), NULL, NULL);
            op->in_frame = current_frame;
            op->in_seg = current_segment;
            linked_list_append(current_segment->operations, op);
            current_segment->left = seg;
            current_segment = seg->right;
            break;
        case A_PRINT_STMT:
            expr = NULL;
            if (node->print_stmt.expression) {
                expr = create_operand(P_TEMP, recurse_IR_tree(node->print_stmt.expression));
            }
            op = create_op(IR_PRINT, expr, NULL, NULL);
            op->in_frame = current_frame;
            op->in_seg = current_segment;
            linked_list_append(current_segment->operations, op);
            break;
        case A_EXPR_STMT:
            recurse_IR_tree(node->expr_stmt.expression);
            break;
        case A_RETURN_STMT:
            expr = NULL;
            if (node->return_stmt.expression) {
                expr = create_operand(P_TEMP, recurse_IR_tree(node->return_stmt.expression));
            }
            op = create_op(IR_RET, expr, NULL, NULL);
            op->in_frame = current_frame;
            op->in_seg = current_segment;
            linked_list_append(current_segment->operations, op);
            break;
        case A_ASSIGN_EXPR:
            expr = create_operand(P_TEMP, recurse_IR_tree(node->assign_expr.expression));
            name = node->assign_expr.identifier->primary_expr.identifier_name;
            if (hash_map_contains(local_variables, name)) {
                id = create_operand(P_TEMP, ((IR_operand *) hash_map_get(local_variables, name))->constant);
                //printf("%s\n", name);
                op = create_op(IR_ASSIGN, id, expr, NULL);
            } else {
                id = create_operand(P_VARIABLE, name);
                op = create_op(IR_ASSIGN, id, expr, NULL);
            }
            op->in_frame = current_frame;
            op->in_seg = current_segment;
            linked_list_append(current_segment->operations, op);
            break;
        case A_LOGICAL_EXPR:
        case A_RELATIONAL_EXPR:
        case A_ARITHMETIC_EXPR:
            IR_operand *left = create_operand(P_TEMP, recurse_IR_tree(node->binary_expr.left));
            IR_operand *right = create_operand(P_TEMP, recurse_IR_tree(node->binary_expr.right));
            enum IR_op_code op_code = AST_op_to_IR_op(node->binary_expr.op);
            
            op = create_op(op_code, create_operand(P_TEMP, temp_counter), left, right);
            op->in_frame = current_frame;
            op->in_seg = current_segment;
            linked_list_append(current_segment->operations, op);
            return temp_counter++;
        case A_UNARY_EXPR:
            // TODO: VERY UNFINISHED
            recurse_IR_tree(node->unary_expr.expression);
            break;
        case A_PRIMARY_EXPR:
            int val = 0;
            switch (node->primary_expr.type) {
                case TYPE_CHAR:
                    val = (int) node->primary_expr.char_value;
                    break;
                case TYPE_INT:
                    val = node->primary_expr.integer_value;
                    break;
                case TYPE_BOOL:
                    val = node->primary_expr.bool_value;
                    break;
                default:
                    break;
            }

            IR_operand *tmp = create_operand(P_TEMP, temp_counter);
            if (node->primary_expr.type == TYPE_IDENTIFIER) {
                name = node->primary_expr.identifier_name;
                if (hash_map_contains(local_variables, name)) {
                    //printf("Prim done\n");
                    return ((IR_operand *) hash_map_get(local_variables, name))->constant; 
                }
                op = create_op(IR_ASSIGN, tmp, create_operand(P_VARIABLE, name), NULL);
            } else {
                op = create_op(IR_ASSIGN, tmp, create_operand(P_CONSTANT, val), NULL);
            }
            op->in_frame = current_frame;
            op->in_seg = current_segment;
            linked_list_append(current_segment->operations, op);
            //printf("Prim done\n");
            return temp_counter++;
            break;
        case A_CALL_EXPR:
            linked_list *ll = node->call_expr.arguments;
            int on_register_params[4] = {0};
            //push regs used in func (save)
            //params
            linked_list_node *lln = ll->head;
            for (int i = 0; i < 4; i++) {
                if (i >= ll->size) { //4, 3, 2, 1 to register
                    break;
                }
                IR_operand *arg = create_operand(P_TEMP, recurse_IR_tree((AST_node *) lln->data));
                op = create_op(IR_PARAM, arg, NULL, NULL);
                on_register_params[i] = arg->constant;
                op->in_frame = current_frame;
                op->in_seg = current_segment;
                linked_list_append(current_segment->operations, op);
                //print_operation(op);
                lln = lln->next;
            }
            int i = ll->size - 4;
            lln = ll->tail;
            while (i > 0) {
                op = create_op(IR_PARAM, create_operand(P_TEMP, recurse_IR_tree((AST_node *) lln->data)), NULL, NULL);
                op->in_frame = current_frame;
                op->in_seg = current_segment;
                linked_list_append(current_segment->operations, op);
                //print_operation(op);
                i--;
                lln = lln->prev;
            }

            //traverse function
            frame *called_func = (frame *) symbol_table_get(current_frame_table, node->call_expr.identifier->primary_expr.identifier_name);
            op = create_op(IR_CALL, create_operand(P_TEMP, temp_counter), create_operand(P_FUNC_CALL, called_func), NULL);
            //print_operation(op);
            op->in_frame = current_frame;
            op->in_seg = current_segment;
            linked_list_append(current_segment->operations, op);

            //params pop
            if (ll->size > 4) {
                op = create_op(IR_POP_PARAM, create_operand(P_CONSTANT, ((ll->size - 4) * 8)), NULL, NULL);
                op->in_frame = current_frame;
                op->in_seg = current_segment;
                linked_list_append(current_segment->operations, op);
                //print_operation(op);
            }
            for (int i = 3; i >= 0; i--) {
                if (!on_register_params[i]) {
                    continue;
                }
                op = create_op(IR_POP_PARAM, create_operand(P_TEMP, on_register_params[i]), NULL, NULL);
                op->in_frame = current_frame;
                op->in_seg = current_segment;
                linked_list_append(current_segment->operations, op);
                //print_operation(op);
            }

            //pop regs used in func (restore)

            return temp_counter++;
        case A_PARAMETER_EXPR:
            
            break;
        default:
            printf("ir.c::recurse_IR_tree: Unknown AST_node kind");
            break;
    }
    return 0;
}

void print_operand(IR_operand *op) {
    switch (op->type) {
        case P_VARIABLE:
            printf("%s", op->variable_name);
            break;
        case P_TEMP:
            printf("T");
        case P_CONSTANT:
            printf("%d", op->constant);
            break;
        case P_FUNC_CALL:
            printf("%s", op->call->name);
            break;
        case P_LABEL:
            // No way to print segment names as of yet
            printf("wuh?");
            break;
    }
}

void print_operation(IR_operation *op) {
    char *name = IR_op_code_to_string(op->op);
    switch (op->op) {
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
        case IR_EQUALS:
        case IR_NEQUALS:
        case IR_LESS:
        case IR_GREATER:
        case IR_LESS_EQ:
        case IR_GREATER_EQ:
        case IR_AND:
        case IR_OR:
            print_operand(op->arg1);
            printf(" <- %s ", name);
            print_operand(op->arg2);
            printf(", ");
            print_operand(op->arg3);
            break;
        case IR_VAR_DECL:
            print_operand(op->arg1);
            if (op->arg2) {
                printf(" <- ");
                print_operand(op->arg2);
            }
            break;
        case IR_ASSIGN:
            print_operand(op->arg1);
            printf(" <- ");
            print_operand(op->arg2);
            break;
        case IR_CALL:
            print_operand(op->arg1);
            printf(" <- ");
            printf("%s ", name);
            print_operand(op->arg2);
            break;
        case IR_GOTO:
            printf("%s", name);
            break;
        case IR_IF:
            printf("%s ", name);
            print_operand(op->arg1);
            break;
        case IR_WHILE:
            printf("%s ", name);
            print_operand(op->arg1);
            break;
        case IR_PRINT:
            printf("%s ", name);
            if (op->arg1) {
                print_operand(op->arg1);
            }
            break;
        case IR_RET:
            printf("%s ", name);
            if (op->arg1) {
                print_operand(op->arg1);
            }
            break;
        case IR_POP_PARAM:
        case IR_PARAM:
            printf("%s ", name);
            print_operand(op->arg1);
            break;
        default:
            

    }
    printf("\n");
}

void print_IR(segment *seg) {
    if (!seg) {
        return;
    }
    //linked_list *keys = get_keys(seg->table);
    /*for (linked_list_node *lln = keys->head; lln != NULL; lln = lln->next) {
        printf("variable: %s\n", (char *) lln->data);
    }*/
    IR_operation *op;
    if (seg->name) {
        printf("%s:\n", seg->name);
    }
    for (linked_list_node *lln = seg->operations->head; lln != NULL; lln = lln->next) {
        op = (IR_operation *) lln->data;
        print_operation(op);
    }
    printf("\n");

    if (seg->operations->size) {
        IR_op_code code = ((IR_operation *) seg->operations->tail->data)->op;
        if (code == IR_GOTO) {
            return;
        }
        print_IR(seg->left);
        print_IR(seg->right);
    }
}

void print_IR_tree(frame *root) {
    if (root->name) {
        printf("%s:\n", root->name);
    }
    print_IR(root->segment);
    for (linked_list_node *lln = root->nested_frames->head; lln != NULL; lln = lln->next) {
        print_IR_tree((frame *) lln->data);
    }
}

frame *create_IR_tree(int *count, AST_node *root) {
    current_frame_table = create_symbol_table(NULL, NULL);
    local_variables = create_hash_map(128);
    frame *global_frame = create_named_frame("/PROGRAM");
    global_frame->segment = create_segment(root->table);
    global_frame->locals = local_variables;
    current_frame = global_frame;
    current_segment = global_frame->segment;
    for (linked_list_node *lln = root->program.modules->head; lln != NULL; lln = lln->next) {
        recurse_IR_tree((AST_node *) lln->data);
    }
    current_frame->last = current_frame->segment;
    if (strcmp("/PROGRAM", current_frame->name) != 0) {
        printf("You serve A LOT of purpose, you should love yourself NOW!\n");
        exit(2);
    }
    liveness(current_frame);
    //print_graph(graph);
    printf("temp_counter: %d\n", temp_counter);
    //print_graph(graph);
    printf("Finished IR creation\n");
    count[0] = temp_counter;
    return global_frame;
}

int add_to_set(linked_list *ll, int temp_num) {
    linked_list_node *lln = linked_list_find(ll, temp_num);
    if (!lln) {
        linked_list_append(ll, temp_num);
        return 1;
    }
    return 0;
}

void set_union(linked_list *ll, linked_list *from) {
    for (linked_list_node *lln = from->head; lln != NULL; lln = lln->next) {
        add_to_set(ll, lln->data);
    }
}

void handle_use_set(IR_operation *op) {
    //printf("Handling a use set of type %s\n", IR_op_code_to_string(op->op));
    IR_op_code code = op->op;
    linked_list *ll = op->use;
    switch(code) {
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
        case IR_EQUALS:
        case IR_NEQUALS:
        case IR_LESS:
        case IR_GREATER:
        case IR_LESS_EQ:
        case IR_GREATER_EQ:
        case IR_AND:
        case IR_OR:
            linked_list_append(ll, op->arg2->constant);
            linked_list_append(ll, op->arg3->constant);
            break;
        case IR_WHILE:
        case IR_IF:
        case IR_PRINT:
        case IR_PARAM:
            linked_list_append(ll, op->arg1->constant);
            break;
        case IR_RET:
            if (op->arg1) {
                linked_list_append(ll, op->arg1->constant);
            }
            break;
        case IR_VAR_DECL:
            if (op->arg2->type != P_CONSTANT && op->arg2->type != P_VARIABLE) {
                linked_list_append(ll, op->arg2->constant);
            }
            break;
        case IR_ASSIGN:
            if (op->arg1->type != P_TEMP) {
                linked_list_append(ll, op->arg2->constant);
            }
            break;
        default:
            break;
    }
}

void handle_def_set(IR_operation *op) {
    //printf("Handling a def set of type %s\n", IR_op_code_to_string(op->op));
    IR_op_code code = op->op;
    linked_list *ll = op->def;
    switch(code) {
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
        case IR_EQUALS:
        case IR_NEQUALS:
        case IR_LESS:
        case IR_GREATER:
        case IR_LESS_EQ:
        case IR_GREATER_EQ:
        case IR_AND:
        case IR_OR:
        case IR_CALL:
            linked_list_append(ll, op->arg1->constant);
            break;
        case IR_ASSIGN:
            if (op->arg1->type == P_TEMP) {
                linked_list_append(ll, op->arg1->constant);
            }
            break;
        default:
            break;
    }
}

linked_list *copy_set(linked_list *ll_from) {
    linked_list *ll = linked_list_new();
    for (linked_list_node *lln = ll_from->head; lln != NULL; lln = lln->next) {
        linked_list_append(ll, lln->data);
    }
    return ll;
}

void liveness(frame *frm) {
    segment *seg;
    int iteration = 0;
    linked_list *new_segments = linked_list_new();
    int change = 1;

    while (change) {
        //printf("Iteration loop\n");
        linked_list_append(new_segments, frm->last);
        change = 0;
        while (new_segments->size != 0) {
            //printf("Segment loop\n");
            seg = (segment *) linked_list_pop_front(new_segments);
            if (seg->operations->size && ((IR_operation *) seg->operations->tail->data)->op == IR_WHILE && seg->left->iteration == iteration) {
                segment *tmp = seg;
                //printf("tmp: %d, seg->left: %d\n", tmp, seg->left);
                seg = seg->left;
                while (tmp != seg->left) {
                    if (seg->operations->size == 0) {
                        seg = seg->left;
                        continue;
                    }
                    //printf("tmp: %d, seg->left: %d, seg->right = %d\t i:%d, si:%d\n", tmp, seg->left, seg->right, iteration, seg->iteration);
                    if (((IR_operation *) seg->operations->tail->data)->op == IR_WHILE) {
                        seg = seg->right;
                    } else {
                        seg = seg->left;
                    }
                }
                //printf("tmp: %d, seg->left: %d\t i:%d, si:%d\n", tmp, seg->left, iteration, seg->iteration);
                linked_list_append(new_segments, tmp);
            }
            if (seg->operations->size && ((IR_operation *) seg->operations->tail->data)->op == IR_IF) {
                int t = 0;
                if (seg->left->iteration == iteration) {
                    linked_list_put_front(new_segments, seg->left);
                    t = 1;
                } else if (seg->right && seg->right->iteration == iteration) {
                    linked_list_put_front(new_segments, seg->right);
                    t = 1;
                }
                if (t) {
                    linked_list_append(new_segments, seg);
                    continue;
                }
            }

            //printf("popped frunk seg->iteration: %d, iteration: %d\n", seg->iteration, iteration);
            if (seg->iteration != iteration) {
                continue;
            }
            //printf("Correct iteration\n");
            seg->iteration++;

            for (linked_list_node *lln = seg->operations->tail; lln != NULL; lln = lln->prev) {
                //printf("Operation loop\n");
                IR_operation *op = ((IR_operation *) lln->data);
                int in_size = op->in->size;
                int out_size = op->out->size;

                // In set work
                //printf("In set work\n");
                linked_list *ll = copy_set(op->out);
                //printf("copied set\n");
                if (op->def->size) {
                    int def = op->def->head->data;
                    //printf("Got number %d\n", def);
                    linked_list_node* ddef = linked_list_find(ll, def);
                    if(ddef){
                        //printf("ddef%d\n", ddef->data);
                        linked_list_remove(ll, ddef);
                        //printf("removed set\n");
                    }
                }
                linked_list_delete(op->in);
                //printf("deleted set\n");
                set_union(ll, op->use);
                //printf("Unionized set\n");
                op->in = ll;
                //printf("In set done!\n");

                // Out set work
                //printf("Out set work\n");
                ll = linked_list_new();
                if (lln == seg->operations->tail) {
                    if (seg->left) {
                        IR_operation *next = (IR_operation *) seg->left->operations->head->data;
                        set_union(ll, next->in);
                        if (seg->right) {
                            next = (IR_operation *) seg->right->operations->head->data;
                            set_union(ll, next->in);
                        }
                    }
                } else {
                    set_union(ll, ((IR_operation *) lln->next->data)->in);
                }
                //printf("Out set done\n");
                linked_list_delete(op->out);
                op->out = ll;

                if (in_size != op->in->size || out_size != op->out->size) {
                    change = 1;
                }
                //printf("oi: %d; oo: %d, ni: %d; no: %d\n", in_size, out_size, op->in->size, op->out->size);
            }

            //printf("Adding segments\n");
            for (linked_list_node *lln = seg->pred->head; lln != NULL; lln = lln->next) {
                linked_list_append(new_segments, (segment *) lln->data);
            }
        }
        iteration++;
    }
    linked_list_delete(new_segments);

    // Liveness analysis of nested frames
    for (linked_list_node *lln = frm->nested_frames->head; lln != NULL; lln = lln->next) {
        liveness((frame *) lln->data);
    }
}