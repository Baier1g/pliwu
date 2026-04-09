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
    segment *tmp = malloc(sizeof(segment));
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
        case P_TEMP:
        case P_CONSTANT:
            op->constant = (int) content;
            break;
        case P_LABEL:
            op->dest = (segment *) content;
        case P_FUNC_CALL:
            op->call = (frame *) content;
    }
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
            local_variables = create_hash_map(128);
            frm->locals = local_variables;
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
                expr = create_operand(P_TEMP, recurse_IR_tree(node->var_decl.expr_stmt));
                hash_map_insert(local_variables, name, expr);
            } else {
                id = create_operand(P_VARIABLE, name);
                expr = create_operand(P_CONSTANT, 0);
                op = create_op(IR_VAR_DECL, id, expr, NULL);
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
            current_segment->right = create_segment(node->if_stmt.else_branch->table);
            current_segment->right->name = IR_generate_label("else", if_c);
            linked_list_append(current_segment->right->pred, current_segment);
            
            IR_operand *if_branch = create_operand(P_LABEL, current_segment->left);
            IR_operand *else_branch = create_operand(P_LABEL, current_segment->right);
            op = create_op(IR_IF, create_operand(P_TEMP, condition), if_branch, else_branch);

            linked_list_append(current_segment->operations, op);
            seg = create_segment(node->table);
            seg->name = IR_generate_label("end_if", if_c);
            linked_list_append(seg->pred, current_segment->left);
            linked_list_append(seg->pred, current_segment->right);
            old_segment = current_segment;

            current_segment = current_segment->left;
            recurse_IR_tree(node->if_stmt.if_branch);
            current_segment->left = seg;
            op = create_op(IR_GOTO, create_operand(P_LABEL, seg), NULL, NULL);
            linked_list_append(current_segment->operations, op);

            current_segment = old_segment->right;
            recurse_IR_tree(node->if_stmt.else_branch);
            current_segment->left = seg;
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
            linked_list_append(current_segment->operations, op);
            
            current_segment = current_segment->left;
            recurse_IR_tree(node->while_loop.block);
            op = create_op(IR_GOTO, create_operand(P_LABEL, seg), NULL, NULL);
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
            linked_list_append(current_segment->operations, op);
            break;
        case A_ASSIGN_EXPR:
            expr = create_operand(P_TEMP, recurse_IR_tree(node->assign_expr.expression));
            name = node->assign_expr.identifier->primary_expr.identifier_name;
            if (hash_map_contains(local_variables, name)) {
                id = create_operand(P_TEMP, ((IR_operand *) hash_map_get(local_variables, name))->constant);
                op = create_op(IR_ASSIGN, id, expr, NULL);
            } else {
                id = create_operand(P_VARIABLE, name);
                op = create_op(IR_ASSIGN, id, expr, NULL);
            }
            linked_list_append(current_segment->operations, op);
            break;
        case A_LOGICAL_EXPR:
        case A_RELATIONAL_EXPR:
        case A_ARITHMETIC_EXPR:
            IR_operand *left = create_operand(P_TEMP, recurse_IR_tree(node->binary_expr.left));
            IR_operand *right = create_operand(P_TEMP, recurse_IR_tree(node->binary_expr.right));
            enum IR_op_code op_code = AST_op_to_IR_op(node->binary_expr.op);

            op = create_op(op_code, create_operand(P_TEMP, temp_counter), left, right);
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
            }

            IR_operand *tmp = create_operand(P_TEMP, temp_counter);
            if (node->primary_expr.type == TYPE_IDENTIFIER) {
                name = node->primary_expr.identifier_name;
                if (hash_map_contains(local_variables, name)) {
                    return ((IR_operand *) hash_map_get(local_variables, name))->constant; 
                }
                op = create_op(IR_ASSIGN, tmp, create_operand(P_VARIABLE, name), NULL);
            } else {
                op = create_op(IR_ASSIGN, tmp, create_operand(P_CONSTANT, val), NULL);
            }
            linked_list_append(current_segment->operations, op);
            return temp_counter++;
            break;
        case A_CALL_EXPR:
            linked_list *ll = node->call_expr.arguments;
            for (linked_list_node *lln = ll->head; lln != NULL; lln = lln->next) {
                IR_operand *arg = create_operand(P_TEMP, recurse_IR_tree((AST_node *) lln->data));
                op = create_op(IR_PARAM, arg, NULL, NULL);
                linked_list_append(current_segment->operations, op);
            }
            frame *called_func = (frame *) symbol_table_get(current_frame_table, node->call_expr.identifier->primary_expr.identifier_name);
            op = create_op(IR_CALL, create_operand(P_TEMP, temp_counter), create_operand(P_FUNC_CALL, called_func), NULL);
            linked_list_append(current_segment->operations, op);
            op = create_op(IR_POP_PARAM, create_operand(P_CONSTANT, node->call_expr.arguments->size * 8), NULL, NULL);
            linked_list_append(current_segment->operations, op);
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
    char *name = IR_op_code_to_string(op->type);
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
    //printf("%s, in_set size: %d, out_set size: %d, def_set size: %d, use_set size: %d\n", name, op->in->size, op->out->size, op->def->size, op->use->size);
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
    linked_list *keys = get_keys(seg->table);
    for (linked_list_node *lln = keys->head; lln != NULL; lln = lln->next) {
        var_info *vi = (var_info *) symbol_table_get(seg->table, (char *) lln->data);
        printf("variable: %s\n", (char *) lln->data);
    }
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
        /*IR_op_code code = ((IR_operation *) seg->operations->tail->data)->op;
        if (code == IR_IF || code == IR_WHILE) {
            segment *left = seg->left;
            for (linked_list_node *lln = left->operations->head; lln != NULL; lln = lln->next) {
                op = (IR_operation *) lln->data;
                print_operation(op);
            }
            printf("\n");
            print_IR(seg->right);
        } else {
            print_IR(seg->left);
            print_IR(seg->right);
        }*/
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

frame *create_IR_tree(AST_node *root) {
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
    RA_graph *graph = create_graph(temp_counter);
    connect_graph(graph, current_frame);
    //print_graph(graph);
    printf("temp_counter: %d\n", temp_counter);
    register_allocation(global_frame, graph);
    printf("temp_counter: %d\n", temp_counter);
    //print_graph(graph);
    printf("Finished IR creation\n");
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
            if (op->arg2->type != P_CONSTANT) {
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
    IR_operation *op;
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
                } else if (seg->right->iteration == iteration) {
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
                IR_op_code code = op->op;

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

RA_graph *create_graph(int num_nodes) {
    RA_graph *graph = (RA_graph *) malloc(sizeof(RA_graph));
    graph->num_nodes = num_nodes;
    graph->adj_matrix = (int **) malloc((num_nodes + 1) * sizeof(int *));
    for (int i = 1; i <= num_nodes + 1; i++) {
        graph->adj_matrix[i] = (int *) calloc(num_nodes + 1, sizeof(int));
        //graph->adj_matrix[i][i] = 1;
    }
    graph->nodes = (RA_node **) malloc(sizeof(RA_node *) * (num_nodes + 1));
    for (int i = 1; i <= graph->num_nodes; i++) {
        *(graph->nodes + i) = create_graph_node(num_nodes);
    }
    return graph;
}

RA_node *create_graph_node(int max_connections) {
    RA_node *node = malloc(sizeof(RA_node));
    node->color = 0;
    node->connections = (int *) calloc(max_connections, sizeof(int));
    node->num_edges = 0;
    return node;
}

void connect_nodes(RA_graph *graph, int temp1, int temp2) {
    RA_node *node1 = graph->nodes[temp1];
    RA_node *node2 = graph->nodes[temp2];
    int in = 0;
    for (int i = 0; i < node1->num_edges; i++) {
        if (node1->connections[i] == temp2) {
            in = 1;
            break;
        }
    }

    if (!in) {
        graph->adj_matrix[temp1][temp2] = graph->adj_matrix[temp2][temp1] = 1;
        node1->connections[node1->num_edges++] = temp2;
        node2->connections[node2->num_edges++] = temp1; 
    }
}

void graph_handle_operation(RA_graph *graph, IR_operation *op) {
    if (op->out->size > 1) {
        int i = 1;
        for (linked_list_node *n = op->out->head; n->next != NULL; n = n->next) {
            i = (int) n->data;
            linked_list_node *ln = n->next;
            while (ln) {
                connect_nodes(graph, i, (int) ln->data);
                ln = ln->next;
            }
        }
    }
}

void connect_graph(RA_graph *graph, frame *frm) {
    linked_list *segments = linked_list_new();
    segment *seg;
    linked_list_append(segments, frm->segment);
    int i = frm->segment->iteration;
    while (segments->size) {
        seg = linked_list_pop_front(segments);
        if (seg->iteration != i) {
            continue;
        }
        seg->iteration++;
        for (linked_list_node *lln = seg->operations->head; lln != NULL; lln = lln->next) {
            IR_operation *op = (IR_operation *) lln->data;
            graph_handle_operation(graph, op);
        }
        if (seg->left) {
            linked_list_append(segments, seg->left);
            if (seg->right) {
                linked_list_append(segments, seg->right);
            }
        }
    }

    for (linked_list_node *lln = frm->nested_frames->head; lln != NULL; lln = lln->next) {
        connect_graph(graph, (frame *) lln->data);
    }
}

void print_graph(RA_graph *graph) {
    for (int i = 1; i < graph->num_nodes; i++) {
        RA_node *node = graph->nodes[i];
        printf("Node T%d: ", i);
        for (int j = 0; j < node->num_edges; j++) {
            printf("%d, ", node->connections[j]);
        }

        printf("| color: %d\n", node->color);
    }
}

void print_adj_matrix(RA_graph *graph) {
    for (int i = 0; i < graph->num_nodes; i++) {
        if (i < 10) {
            printf("%d  ", i);
        } else {
            printf("%d ", i);
        }
    }
    printf("\n");
    for (int i = 1; i < graph->num_nodes; i++) {
        if (i < 10) {
            printf("%d  ", i);
        } else {
            printf("%d ", i);
        }
        for (int j = 1; j < graph->num_nodes; j++) {
            if (j < 10) {
                printf(" %d ", graph->adj_matrix[i][j]);
            } else {
                printf(" %d ", graph->adj_matrix[i][j]);
            }
        }
        printf("\n");
    }
}

void RA_disconnect_node(RA_graph *graph, int t1) {
    RA_node *node = graph->nodes[t1];
    for (int i = 0; i < node->num_edges; i++) {
        int tmp = node->connections[i];
        //graph->adj_matrix[t1][tmp] = graph->adj_matrix[tmp][t1] = 0;
        graph->nodes[tmp]->num_edges--;
    }
    node->num_edges = 0;
}

void RA_simplify(RA_graph *graph, int *simple, int *spill) {
    int spill_count, simple_count;
    spill_count = simple_count = 0;
    for (int i = 1; i < graph->num_nodes; i++) {
        if (graph->nodes[i]->num_edges < MAX_REG) {
            RA_disconnect_node(graph, i);
            simple[simple_count++] = i;
        } else {
            spill[spill_count++] = i;
        }
    }
}

/*
 * colour selection part of register allocation.
 * Returns the number of actual spill nodes
 */
int RA_select(RA_graph *graph, int *simple, int* potential_spill, int *spill) {
    int current_color = 1;
    int spill_count = 0;
    RA_node *node, *tmp;

    int i = 0;
    int curr = 0;
    while ((curr = simple[i]) != 0) {
        node = graph->nodes[curr];
        int colors[MAX_REG + 1] = {};
        int j = curr;
        /*for (int i = 0; node->connections[i] != 0; i++) {

        }*/
        while (j > 0) {
            if (graph->adj_matrix[curr][j] == 1) {
                connect_nodes(graph, curr, j);
                colors[graph->nodes[j]->color]++;
            }
            j--;
        }
        int color = 0;
        for (int k = 1; k < MAX_REG + 1; k++) {
            if (colors[k] == 0) {
                color = k;
                break;
            }
        }
        if (color != 0) {
            node->color = color;
        } else {
            spill[spill_count++] = curr;
        }
        i++;
    }
    i = (temp_counter - 2) - i;
    while (i >= 0 && (curr = potential_spill[i]) != 0) {
        node = graph->nodes[curr];
        int colors[MAX_REG + 1] = {};
        int j = graph->num_nodes;
        while (j > 0) {
            if (graph->adj_matrix[curr][j] == 1) {
                connect_nodes(graph, curr, j);
                colors[graph->nodes[j]->color]++;
            }
            j--;
        }
        int color = 0;
        for (int k = 1; k <= MAX_REG; k++) {
            if (colors[k] == 0) {
                color = k;
                break;
            }
        }
        if (color != 0) {
            node->color = color;
        } else {
            spill[spill_count++] = curr;
        }
        i--;
    }

    return spill_count;
}

void rewrite_segment(segment *seg, int spilled_node, int defined, char *var_name) {
    if (!seg) {
        return;
    }
    linked_list_node *new, *tmp;
    IR_operand *op1, *op2, *op3;
    IR_operation *operation, *current_op;
    for (linked_list_node *lln = seg->operations->head; lln != NULL; lln = lln->next) {
        current_op = (IR_operation *) lln->data;
        //print_operation(current_op);
        linked_list_node *tmp = linked_list_find(current_op->in, spilled_node);
        if (tmp) {
            linked_list_remove(current_op->in, tmp);
        }
        tmp = linked_list_find(current_op->out, spilled_node);
        if (tmp) {
            linked_list_remove(current_op->out, tmp);
        }
        
        if (current_op->arg1 && current_op->arg1->type == P_TEMP && current_op->arg1->constant == spilled_node) {
            if (!defined) {
                current_op->arg1->type = P_VARIABLE;
                current_op->arg1->variable_name = var_name;
                //op1 = create_operand(P_VARIABLE, var_name);
                //op2 = create_operand(P_TEMP, spilled_node);
                //operation = create_op(IR_VAR_DECL, op1, op2, NULL);
                //linked_list_append(operation->use, spilled_node);
                //linked_list_put_next(seg->operations, lln, operation);
                //lln = lln->next;
                defined = 1;
                continue;
            } else {
                if (current_op->op == IR_IF || current_op->op == IR_WHILE) {
                    tmp = lln->prev;
                    linked_list_append(current_op->in, temp_counter);

                    op1 = create_operand(P_TEMP, temp_counter);
                    op2 = create_operand(P_VARIABLE, var_name);
                    operation = create_op(IR_ASSIGN, op1, op2, NULL);
                    linked_list_copy_to(((IR_operation *)tmp->data)->out, operation->in);
                    linked_list_copy_to(current_op->in, operation->out);

                    linked_list_put_next(seg->operations, tmp, operation);
                    current_op->arg1->constant = temp_counter++;
                } else {
                    current_op->arg1->type = P_VARIABLE;
                    current_op->arg1->variable_name = var_name;
                }
            }        
        }
        int p2, p3;
        if ((p2 = (current_op->arg2 && current_op->arg2->type == P_TEMP && current_op->arg2->constant == spilled_node))
        || (p3 = (current_op->arg3 && current_op->arg3->type == P_TEMP && current_op->arg3->constant == spilled_node))) {
            if (current_op->op == IR_ASSIGN) {
                current_op->arg2->type = P_VARIABLE;
                current_op->arg2->variable_name = var_name;
                continue;
            }
            
            op1 = create_operand(P_TEMP, temp_counter);
            op2 = create_operand(P_VARIABLE, var_name);
            operation = create_op(IR_ASSIGN, op1, op2, NULL);
            
            printf("WE MADE IT\n");
            tmp = lln->prev;
            if (!tmp) {
                linked_list_copy_to(current_op->in, operation->in);
                linked_list_append(current_op->in, temp_counter);
                linked_list_copy_to(current_op->in, operation->out);
                linked_list_put_front(seg->operations, operation);
            } else {
                linked_list_append(current_op->in, temp_counter);
                linked_list_copy_to(((IR_operation *)lln->prev->data)->out, operation->in);
                linked_list_copy_to(current_op->in, operation->out);
                linked_list_put_next(seg->operations, lln->prev, operation);
            }
            if (p2) {
                current_op->arg2->constant = temp_counter;
            }
            if (p3) {
                current_op->arg3->constant = temp_counter;
            }
            temp_counter++;
        }
    }
    printf("In rewrite_segment\n");
    linked_list_node *bruh = seg->operations->tail;
    if (bruh && !(((IR_operation *) bruh->data)->op == IR_GOTO)) {
        rewrite_segment(seg->left, spilled_node, defined, var_name);
        //printf("recursing right\n");
        rewrite_segment(seg->right, spilled_node, defined, var_name);
    }

    return;
}

/*
 * UNFINISHED AND VERY NECESSARY
 */
void rewrite_program(frame *frm, int* spilled_nodes, int count) {
    if (spilled_nodes[count] == 0) {
        return;
    }
    frame *current_frame;
    linked_list *new_frames = linked_list_new();
    linked_list_append(new_frames, frm);
    char *name;
    int key_found = 0;
    //printf("in rewrite\n");
    while (!key_found) {
        if (!new_frames->size) {
            return;
        }
        //printf("in loop\n");
        current_frame = linked_list_pop_front(new_frames);
        linked_list *keys = current_frame->locals->keys;
        for (linked_list_node *lln = keys->head; lln != NULL; lln = lln->next) {
            IR_operand *tmp = (IR_operand *) hash_map_get(current_frame->locals, (char *) lln->data);
            if (tmp && tmp->constant == spilled_nodes[count]) {
                printf("Key found!\n");
                key_found = 1;
                name = (char *) lln->data;
                break;
            }
        }
        if (key_found) {
            break;
        }
        //printf("No key found, checking nested frames\n");
        for (linked_list_node *lln = current_frame->nested_frames->head; lln != NULL; lln = lln->next) {
            linked_list_append(new_frames, (frame *) lln->data);
        }
    } 
    rewrite_segment(current_frame->segment, spilled_nodes[count], 0, name);
    count++;
    rewrite_program(frm, spilled_nodes, count);
    return;   
}

void register_allocation(frame *program, RA_graph *graph) {
    int count = 0;
    int *simple_nodes = calloc(graph->num_nodes + 1, sizeof(int));
    int *potential_spill = calloc(graph->num_nodes + 1, sizeof(int));
    int *actual_spill = calloc(graph->num_nodes + 1, sizeof(int));
    
    /* PSEUDO PROCEDURE
    K: Number of registers
    simplify graph: 
        add all nodes of degree < K to simple_nodes, rest to potential spill
    select colors: 
        go through all simple nodes and color them, afterwards go through all nodes in potential spill.
        if a potential spill node is uncolourable, add them to actual spill, but continue
    while actual spill nodes exist:
        rewrite program to push and pop from stack, adding new temporaries where needed.
        rebuild graph from program, simplify and select again.
    */

    RA_simplify(graph, simple_nodes, potential_spill);
   
    int spill = RA_select(graph, simple_nodes, potential_spill, actual_spill);
    count++;
    while (spill) {
        printf("spill node is: %d\n", actual_spill[0]);
        rewrite_program(program, actual_spill, 0);
        // KILL GWAPH UwU
        graph = create_graph(temp_counter);
        connect_graph(graph, program);
        //print_graph(graph);
        free(simple_nodes);
        free(potential_spill);
        free(actual_spill);
        *simple_nodes = calloc(graph->num_nodes + 1, sizeof(int));
        *potential_spill = calloc(graph->num_nodes + 1, sizeof(int));
        *actual_spill = calloc(graph->num_nodes + 1, sizeof(int));
        RA_simplify(graph, simple_nodes, potential_spill);
   
        spill = RA_select(graph, simple_nodes, potential_spill, actual_spill);
        count++;
        //register_allocation(program, graph);
    }
    print_graph(graph);
    print_IR_tree(program);
    printf("Spills: %d, runs: %d\n", spill, count);
    //free(simple_nodes);
    //free(potential_spill);
    //free(actual_spill);
    return;
}