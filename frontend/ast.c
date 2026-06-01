#include "ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int indents = 0;

void print_indents(void);
void kill_ll(linked_list*);

pos create_pos(int start, int line) {
    pos pos;
    pos.startchar = start;
    pos.line = line;
    return pos;
}

var_info *create_var_info(int nesting_depth, int func_nesting_depth) {
    var_info *tmp = calloc(1, sizeof(var_info));
    if (!tmp) {
        printf("ast.c::create_var_info: calloc FAILED\n");
        return NULL;
    }
    tmp->nesting_depth = nesting_depth;
    tmp->func_nesting_depth = func_nesting_depth; 
    return tmp;
}

AST_node *create_unary_node(int startchar, int line, kind node_kind, void *a) {
    AST_node *node = malloc(sizeof(AST_node));
    if (!node) {
        return NULL;
    }
    node->pos = create_pos(startchar, line);
    node->kind = node_kind;
    node->table = NULL;
    switch (node_kind) {
        case A_PROGRAM:
            node->program.modules = a;
            break;
        case A_MODULE:
            node->module.module_declarations = a;
            break;
        case A_EXPR_STMT:
            node->expr_stmt.expression = a;
            break;
        case A_PRINT_STMT:
            node->print_stmt.expression = a;
            break;
        case A_BLOCK_STMT:
            node->block.stmt_list = a;
            break;
        case A_RETURN_STMT:
            node->return_stmt.expression = a;
            break;
        default:
            printf("ast.c::create_unary_node: Unexpected kind %d\n", node_kind);
            break;
        }
    return node;
}

AST_node *create_binary_node(int startchar, int line, kind node_kind, void *a, void *b) {
    AST_node *node = malloc(sizeof(AST_node));
    node->pos = create_pos(startchar, line);
    node->kind = node_kind;
    node->table = NULL;
    switch (node_kind) {
        case A_PRIMARY_EXPR:
            node->primary_expr.type = (data_type) a;
            switch (node->primary_expr.type) {
                case TYPE_INT:
                    node->primary_expr.integer_value = (long) b;
                    break;
                case TYPE_CHAR:
                    node->primary_expr.char_value = (char) b;
                    break;
                case TYPE_BOOL:
                    node->primary_expr.bool_value = (short) b;
                    break;
                case TYPE_IDENTIFIER:
                    size_t len = strlen((char*) b);
                    node->primary_expr.identifier_name = calloc(len + 1, sizeof(char));
                    strncpy(node->primary_expr.identifier_name, b, len);
                    break;
                default:
                    break;
            }
            break;
        case A_UNARY_EXPR:
            node->unary_expr.op = (unary_op) a;
            node->unary_expr.expression = b;
            break;
        case A_WHILE_LOOP:
            node->while_loop.condition = a;
            node->while_loop.block = b;
            break;
        case A_ASSIGN_EXPR:
            node->assign_expr.identifier = a;
            node->assign_expr.expression = b;
            break;
        case A_PARAMETER_EXPR:
            node->parameter.type = (data_type) a;
            node->parameter.identifier = b;
            node->parameter.array = 0;
            break;
        case A_CALL_EXPR:
            node->call_expr.identifier = a;
            node->call_expr.arguments = b;
            break;
        case A_INDEX_EXPR:
            node->indexing.identifier = a;
            node->indexing.indices = (linked_list *) b;
            break;
        default:
            printf("ast.c::create_binary_node: Unexpected kind %d\n", node_kind);
            break;
    }
    return node;
}

AST_node *create_ternary_node(int startchar, int line, kind node_kind, void* a, void* b, void *c) {
    AST_node *node = malloc(sizeof(AST_node));
    if (!node) {
        return NULL;
    }
    node->pos = create_pos(startchar, line);
    node->kind = node_kind;
    node->table = NULL;
    switch (node_kind) {
        case A_IF_STMT:
            node->if_stmt.condition = a;
            node->if_stmt.if_branch = b;
            node->if_stmt.else_branch = c;
            break;
        case A_ARITHMETIC_EXPR:
        case A_LOGICAL_EXPR:
        case A_RELATIONAL_EXPR:
            node->binary_expr.left = a;
            node->binary_expr.op = (binary_op) b;
            node->binary_expr.right = c;
            break;
        case A_VAR_DECL:
            node->var_decl.type = (data_type) a;
            node->var_decl.identifier = b;
            node->var_decl.expr_stmt = c;
            break;
        case A_PRIMARY_EXPR:
            node->primary_expr.type = (data_type) a;
            node->primary_expr.string.value = (char *) calloc((int) c, sizeof(char));
            strncpy(node->primary_expr.string.value, (char *) b, c);
            node->primary_expr.string.length = (int) c;
            break;
        default:
            printf("ast.c::create_ternary_node: Unexpected kind %d\n", node_kind);
            break;
        }
    return node;
}

int ds_helper(AST_node* anode, linked_list *sizes, linked_list *values, int depth){
    if (depth >= sizes->size){
        if ((unsigned int) values->head > 30) {
            printf("ast.c::arr: initialisation array too deep\n");
            return -1;
        }
        return 0;
    }
    if (values->size <= 0) {
        printf("ast.c::arr: wrong declared depth\n");
        return -1;
    }

    //check current dim
    linked_list_node *s = sizes->head;
    for (int i = 0; i < depth; i++) {
        s = s->next;
    }
    if(s->data == NULL){
        //set 
        s->data = values->size;
    } else {
        //validate
        if ((int) s->data != values->size) {
            printf("ast.c::arr: wrong sizes for initialized array\n"); //anode err print
        }
    }
    
    //check next dim
    for (linked_list_node *lln = values->head; lln != NULL; lln = lln->next) {
        if (1){
            if (ds_helper(anode, sizes, (linked_list *) lln->data, depth+1)) {
                return -1;
            }
        } else {
            printf("ast.c::arr: Array missing from initializor\n"); //anode err print
            return -1;
        }
    }
    return 0;
}

int define_sizes(AST_node* anode, linked_list *sizes, linked_list *values){
    if (ds_helper(anode, sizes, values, 0)) {
        return -1;
    }
    for (linked_list_node *lln = sizes->head; lln != NULL; lln = lln->next) {
        lln->data = create_binary_node(anode->pos.startchar, anode->pos.line, A_PRIMARY_EXPR, TYPE_INT, (long) lln->data);
    }
    return 0;
}

AST_node *create_quaternary_node(int startchar, int line, kind node_kind, void *a, void *b, void *c, void *d) {
    AST_node *node = malloc(sizeof(AST_node));
    if (!node) {
        return NULL;
    }
    node->pos = create_pos(startchar, line);
    node->kind = node_kind;
    node->table = NULL;
    switch (node_kind) {
        case A_FUNC_DEF:
            node->func_def.return_type = (data_type) a;
            node->func_def.identifier = b;
            node->func_def.parameters = c;
            node->func_def.function_block = d;
            break;
        case A_ARRAY_DECL:
            node->array_decl.type = (data_type) a;
            node->array_decl.identifier = b;
            node->array_decl.sizes = c;
            if (d) {
                if (define_sizes(node, c, d)) {
                    return NULL;
                }
            }
            node->array_decl.values = d;
            break;
        default:
            printf("ast.c::create_quaternary_node: Unexpected kind %d\n", node_kind);
            break;
        }
    return node;
}

char *kind_enum_to_string(kind type) {
    switch(type) {
        case A_PROGRAM:
            return "PROGRAM";
        case A_MODULE:
            return "MODULE";
        case A_FUNC_DEF:
            return "Function definition";
        case A_VAR_DECL:
            return "Variable declaration";
        case A_ARRAY_DECL:
            return "Array declaration";
        case A_BLOCK_STMT:
            return "Block statement";
        case A_IF_STMT:
            return "If statement";
        case A_WHILE_LOOP:
            return "While loop";
        case A_PRINT_STMT:
            return "Print statement";
        case A_EXPR_STMT:
            return "Expression statement";
        case A_RETURN_STMT:
            return "Return statement";
        case A_ASSIGN_EXPR:
            return "Assignment expression";
        case A_LOGICAL_EXPR:
            return "Logical expression";
        case A_RELATIONAL_EXPR:
            return "Relational expression";
        case A_ARITHMETIC_EXPR:
            return "Arithmetic expression";
        case A_UNARY_EXPR:
            return "Unary expression";
        case A_PRIMARY_EXPR:
            return "Primary expression";
        case A_CALL_EXPR:
            return "Call expression";
        case A_PARAMETER_EXPR:
            return "Parameter expression";
        case A_INDEX_EXPR:
            return "Index expression";
        default:
            return "ast.c::kind_enum_to_string: Unknown kind";
    }
}

char *binary_op_enum_to_string(binary_op operand) {
    switch(operand) {
        case A_LESS:
            return "<";
        case A_GREATER:
            return ">";
        case A_LESS_EQ:
            return "<=";
        case A_GREATER_EQ:
            return ">=";
        case A_EQUALS:
            return "==";
        case A_NEQUALS:
            return "!=";
        case A_ASSIGN:
            return "=";
        case A_ADD:
            return "+";
        case A_SUB:
            return "-";
        case A_MULT:
            return "*";
        case A_DIV:
            return "/";
        case A_AND:
            return "AND";
        case A_OR:
            return "OR";
        default:
            return "ast.c::binary_op_enum_to_string: Unknown operator";
    }
}

void kill_tree(AST_node* node) {
    if (!node) {
        return;
    }
    kind type = node->kind;
    if (node->table && node->kind != A_PRINT_STMT) {
        destroy_symbol_table(node->table);
    }
    switch (type) {
        case A_PROGRAM:
            kill_ll(node->program.modules);
            break;
        case A_MODULE:
            kill_ll(node->module.module_declarations);
            break;
        case A_FUNC_DEF:
            kill_tree(node->func_def.identifier);
            kill_ll(node->func_def.parameters);
            kill_tree(node->func_def.function_block);
            break;
        case A_VAR_DECL:
            kill_tree(node->var_decl.identifier);
            kill_tree(node->var_decl.expr_stmt);
            break;
        case A_ARRAY_DECL:
            kill_tree(node->array_decl.identifier);
            kill_ll(node->array_decl.sizes);
            if (node->array_decl.values) {
                // MORE ROBUST FREEING NEEDED, doesn't work on multidimensional arrays
                // WHO ELSE UP SEGGING THEY FAULT RIGHT NOW, THIS KILLS THE COMPILER
                //kill_ll(node->array_decl.values);
            }
            break;
        case A_BLOCK_STMT:
            kill_ll(node->block.stmt_list);
            break;
        case A_IF_STMT:
            kill_tree(node->if_stmt.condition);
            kill_tree(node->if_stmt.if_branch);
            kill_tree(node->if_stmt.else_branch);
            break;
        case A_WHILE_LOOP:
            kill_tree(node->while_loop.condition);
            kill_tree(node->while_loop.block);
            break;
        case A_PRINT_STMT:
            kill_tree(node->print_stmt.expression);
            break;
        case A_EXPR_STMT:
            kill_tree(node->expr_stmt.expression);
            break;
        case A_RETURN_STMT:
            kill_tree(node->return_stmt.expression);
            break;
        case A_ASSIGN_EXPR:
            kill_tree(node->assign_expr.identifier);
            kill_tree(node->assign_expr.expression);
            break;
        case A_LOGICAL_EXPR:
        case A_RELATIONAL_EXPR:
        case A_ARITHMETIC_EXPR:
            kill_tree(node->binary_expr.left);
            kill_tree(node->binary_expr.right);
            break;
        case A_UNARY_EXPR:
            kill_tree(node->unary_expr.expression);
            break;
        case A_PRIMARY_EXPR:
            if (node->primary_expr.type == TYPE_IDENTIFIER) {
                free(node->primary_expr.identifier_name);
            } else if (node->primary_expr.type == TYPE_STRING) {
                free(node->primary_expr.string.value);
            }
            break;
        case A_CALL_EXPR:
            kill_tree(node->call_expr.identifier);
            kill_ll(node->call_expr.arguments);
            break;
        case A_PARAMETER_EXPR:
            kill_tree(node->parameter.identifier);
            break;
        case A_INDEX_EXPR:
            kill_ll(node->indexing.indices);
            kill_tree(node->indexing.identifier);
            break;
        default:
            printf("ast.c::kill_tree: Unknown AST kind\n");
            return;
    }
    free(node);
    return;
}

void AST_printer(AST_node *node) {
    if (!node) {
        return;
    }
    print_indents();
    printf("%s:\n", kind_enum_to_string(node->kind));
    switch(node->kind) {
        case A_PROGRAM:
            {
                indents++;
                linked_list_node *ptr = node->program.modules->head;
                while (ptr) {
                    AST_printer(ptr->data);
                    ptr = ptr->next;
                }
                indents--;
            }
            break;
        case A_MODULE:
            {
                indents++;
                linked_list_node *ptr = node->module.module_declarations->head;
                while (ptr) {
                    AST_printer(ptr->data);
                    ptr = ptr->next;
                }
                indents--;
            }
            break;
        case A_FUNC_DEF:
            indents++;
            print_indents();
            printf("Return type: %d\n", node->func_def.return_type);
            AST_printer(node->func_def.identifier);
            linked_list_node *ptrs = node->func_def.parameters->head;
            while (ptrs) {
                AST_printer(ptrs->data);
                ptrs = ptrs->next;
            }
            AST_printer(node->func_def.function_block);
            indents--;
            break;
        case A_PARAMETER_EXPR:
            indents++;
            print_indents();
            printf("Type: %d\n", node->parameter.type);
            AST_printer(node->parameter.identifier);
            indents--;
            break;
        case A_VAR_DECL:
            indents++;
            print_indents();
            printf("Type: %d\n", node->var_decl.type);
            AST_printer(node->var_decl.identifier);
            AST_printer(node->var_decl.expr_stmt);
            indents--;
            break;
        case A_ARRAY_DECL:
            indents++;
            print_indents();
            printf("Type: %d\n", node->array_decl.type);
            AST_printer(node->array_decl.identifier);
            print_indents();
            printf("Sizes:\n");
            indents++;
            for (linked_list_node *lln = node->array_decl.sizes->head; lln != NULL; lln = lln->next) {
                AST_printer((AST_node *) lln->data);
            }
            indents--;
            
            indents--;
            break;
        case A_BLOCK_STMT:
            linked_list_node *ptr = node->block.stmt_list->head;
            indents++;
            while (ptr) {
                AST_printer(ptr->data);
                ptr = ptr->next;
            }
            indents--;
            break;
        case A_IF_STMT:
            indents++;
            AST_printer(node->if_stmt.condition);
            AST_printer(node->if_stmt.if_branch);
            AST_printer(node->if_stmt.else_branch);
            indents--;
            break;
        case A_WHILE_LOOP:
            indents++;
            AST_printer(node->while_loop.condition);
            AST_printer(node->while_loop.block);
            indents--;
            break;
        case A_PRINT_STMT:
            indents++;
            AST_printer(node->print_stmt.expression);
            indents--;
            break;
        case A_EXPR_STMT:
            break;
        case A_RETURN_STMT:
            indents++;
            AST_printer(node->return_stmt.expression);
            indents--;
            break;
        case A_ASSIGN_EXPR:
            indents++;
            AST_printer(node->assign_expr.identifier);
            AST_printer(node->assign_expr.expression);
            indents--;
            break;
        case A_LOGICAL_EXPR:
        case A_RELATIONAL_EXPR:
        case A_ARITHMETIC_EXPR:
            indents++;
            AST_printer(node->binary_expr.left);
            print_indents();
            printf("Operator: %s\n", binary_op_enum_to_string(node->binary_expr.op));
            AST_printer(node->binary_expr.right);
            indents--;
            break;
        case A_UNARY_EXPR:
            indents++;
            print_indents();
            printf("Operator: %c\n", node->unary_expr.op == A_NEG ? '!' : '-');
            AST_printer(node->unary_expr.expression);
            indents--;
            break;
        case A_PRIMARY_EXPR:
            indents++;
            print_indents();
            printf("Type: %d\n", node->primary_expr.type);
            print_indents();
            switch (node->primary_expr.type) {
                case TYPE_INT:
                    printf("Literal: %ld\n", node->primary_expr.integer_value);
                    break;
                case TYPE_CHAR:
                    printf("Literal: %c\n", node->primary_expr.char_value);
                    break;
                case TYPE_BOOL:
                    printf("Literal: %s\n", node->primary_expr.bool_value ? "True" : "False");
                    break;
                case TYPE_IDENTIFIER:
                    printf("Literal: %s\n", node->primary_expr.identifier_name);
                    break;
                case TYPE_VOID:
                    printf("Literal: None\n");
                    break;
                case TYPE_STRING:
                    printf("String:\n");
                    indents++;
                    print_indents();
                    printf("Literal: \"%s\"\n", node->primary_expr.string.value);
                    print_indents();
                    printf("Length: %d\n", node->primary_expr.string.length);
                    indents--;
                    break;
                default:
                    printf("ast.c::AST_printer::primary: Unknown primary type\n");
                    break;
            }
            indents--;
            break;
        case A_CALL_EXPR:
            indents++;
            AST_printer(node->call_expr.identifier);
            linked_list_node *llpeter = node->call_expr.arguments->head;
            while (llpeter) {
                AST_printer(llpeter->data);
                llpeter = llpeter->next;
            }
            indents--;
            break;
        case A_INDEX_EXPR:
            indents++;
            AST_printer(node->call_expr.identifier);
            print_indents();
            printf("indices:\n");
            indents++;
            for (linked_list_node *lln = node->indexing.indices->head; lln != NULL; lln = lln->next) {
                AST_printer(lln->data);
            }
            indents--;
            indents--;
        break;
        default:
            printf("ast.c::AST_printer: Unknown AST kind\n");
            break;
    }
}

void print_indents() {
    for (int i = 0; i < indents; i++) {
        printf("|   ");
    }
}

void kill_ll(linked_list *ll) {
    linked_list_node *curr, *next;
    curr = ll->head;
    while (curr) {
        next = curr->next;
        kill_tree(curr->data);
        free(curr);
        curr = next;
    }
    free(ll);
}