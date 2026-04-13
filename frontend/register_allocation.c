#include "register_allocation.h"

int temp_c;
RA_graph *glob_graph;

RA_graph *create_graph(int num_nodes) {
    RA_graph *graph = (RA_graph *) malloc(sizeof(RA_graph));
    graph->num_nodes = num_nodes;
    graph->adj_matrix = (int **) malloc((num_nodes + 1) * sizeof(int *));
    graph->adj_matrix[0] = NULL;
    for (int i = 1; i <= num_nodes; i++) {
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
    //printf("connect_nodes: invalid temp_val T_one: %d, T_two: %d, max temp: %d\n", temp1, temp2, graph->num_nodes);
    RA_node *node1 = graph->nodes[temp1];
    RA_node *node2 = graph->nodes[temp2];
    int in = 0;
    for (int i = 0; i < node1->num_edges; i++) {
        if (node1->connections[i] == temp2) {
            in = 1;
            break;
        }
    }
    //printf("checked connections\n");

    if (!in) {
        //printf("in if\n");
        graph->adj_matrix[temp1][temp2] = graph->adj_matrix[temp2][temp1] = 1;
        //printf("adj_matrix set\n");
        node1->connections[node1->num_edges++] = temp2;
        //printf("node 1 connections set. node2->num_edges: %d\n", node2->num_edges);
        node2->connections[node2->num_edges++] = temp1;
        //printf("node 2 connections set\n");
    }
    //printf("MADE IT OUT THE CONNECT_NODES\n");
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

void kill_graph(RA_graph *graph) {
    for (int i = 1; i < graph->num_nodes + 1; i++) {
        free(graph->adj_matrix[i]);
        free(graph->nodes[i]->connections);
        free(graph->nodes[i]);
    }
    free(graph->nodes);
    free(graph->adj_matrix);
    free(graph);
}

void RA_disconnect_node(RA_graph *graph, int t1) {
    RA_node *node = graph->nodes[t1];
    for (int i = 0; i < node->num_edges; i++) {
        int tmp = node->connections[i];
        //graph->adj_matrix[t1][tmp] = graph->adj_matrix[tmp][t1] = 0;
        if (graph->nodes[tmp]->num_edges - 1 < 0) {
            printf("RA_disconnect_node: something went wrong\n");
        } else {
            RA_node *temp = graph->nodes[tmp];
            for (int j = 0; j < temp->num_edges; j++) {
                if (temp->connections[j] == t1) {
                    int k = j + 1;
                    while (k <= temp->num_edges) {
                        temp->connections[k - 1] = temp->connections[k];
                        k++;
                    }
                    graph->nodes[tmp]->num_edges--;
                    break;
                }
            }
        }
    }
    node->num_edges = 0;
}

void RA_simplify(RA_graph *graph, int *simple, int *spill) {
    printf("in RA_simplify\n");
    int spill_count, simple_count;
    spill_count = simple_count = 0;
    for (int i = 1; i <= graph->num_nodes; i++) {
        if (graph->nodes[i]->num_edges < MAX_REG) {
            RA_disconnect_node(graph, i);
            simple[simple_count++] = i;
        } else {
            spill[spill_count++] = i;
        }
    }
}

void sort_nodes(RA_graph *graph, int *arr) {
    int i = 1;
    int curr = 0;
    while ((curr = arr[i]) != 0) {
        int j = i - 1;
        while (j >= 0 && graph->nodes[arr[j]]->num_edges > graph->nodes[curr]->num_edges) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = curr;
        i++;
    }
}

/*
 * colour selection part of register allocation.
 * Returns the number of actual spill nodes
 */
int RA_select(RA_graph *graph, int *simple, int* potential_spill, int *spill) {
    printf("In RA_select\n");
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
    //i = (temp_c - 1) - i;
    i = 0;
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
    printf("RA_select finished\n");
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
                    linked_list_append(current_op->in, temp_c);

                    op1 = create_operand(P_TEMP, temp_c);
                    op2 = create_operand(P_VARIABLE, var_name);
                    operation = create_op(IR_ASSIGN, op1, op2, NULL);
                    linked_list_copy_to(((IR_operation *)tmp->data)->out, operation->in);
                    linked_list_copy_to(current_op->in, operation->out);

                    linked_list_put_next(seg->operations, tmp, operation);
                    current_op->arg1->constant = temp_c++;
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
            
            op1 = create_operand(P_TEMP, temp_c);
            op2 = create_operand(P_VARIABLE, var_name);
            operation = create_op(IR_ASSIGN, op1, op2, NULL);
            
            //printf("WE MADE IT\n");
            tmp = lln->prev;
            if (!tmp) {
                linked_list_copy_to(current_op->in, operation->in);
                linked_list_append(current_op->in, temp_c);
                linked_list_copy_to(current_op->in, operation->out);
                linked_list_put_front(seg->operations, operation);
            } else {
                linked_list_append(current_op->in, temp_c);
                linked_list_copy_to(((IR_operation *)lln->prev->data)->out, operation->in);
                linked_list_copy_to(current_op->in, operation->out);
                linked_list_put_next(seg->operations, lln->prev, operation);
            }
            if (p2) {
                current_op->arg2->constant = temp_c;
            }
            if (p3) {
                current_op->arg3->constant = temp_c;
            }
            temp_c++;
        }
    }
    //printf("In rewrite_segment\n");
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
        printf("No key found, checking nested frames\n");
        for (linked_list_node *lln = current_frame->nested_frames->head; lln != NULL; lln = lln->next) {
            linked_list_append(new_frames, (frame *) lln->data);
        }
    } 
    rewrite_segment(current_frame->segment, spilled_nodes[count], 0, name);
    count++;
    rewrite_program(frm, spilled_nodes, count);
    return;   
}

RA_graph *register_allocation(int temps, frame *program) {
    int count = 0;
    temp_c = temps;
    RA_graph *graph = create_graph(temp_c);
    glob_graph = graph;

    connect_graph(graph, program);

    int *simple_nodes = (int *) calloc(graph->num_nodes + 1, sizeof(int));
    int *potential_spill = (int *) calloc(graph->num_nodes + 1, sizeof(int));
    int *actual_spill = (int *) calloc(graph->num_nodes + 1, sizeof(int));
    
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
    for (int i = 1; i < graph->num_nodes; i++) {
        printf("Node %d has %d edges\n", i, graph->nodes[i]->num_edges);
    }
    sort_nodes(graph, potential_spill);
    int i = 0;
    while (potential_spill[i] != 0) {
        printf("%d, ", graph->nodes[potential_spill[i++]]->num_edges);
    }
    printf("\n");
   
    int spill = RA_select(graph, simple_nodes, potential_spill, actual_spill);
    count++;
    //print_graph(graph);
    while (spill) {
        printf("Spills: %d, runs: %d\n", spill, count);
        printf("spill node is: %d and temp count is: %d\n", actual_spill[0], temp_c);
        rewrite_program(program, actual_spill, 0);
        free(simple_nodes);
        free(potential_spill);
        free(actual_spill);
        // KILL GWAPH UwU
        kill_graph(graph);
        graph = NULL;
        graph = create_graph(temp_c);
        glob_graph = graph;
        connect_graph(graph, program);
        //print_graph(graph);
        simple_nodes = (int *) calloc(graph->num_nodes + 1, sizeof(int));
        potential_spill = (int *) calloc(graph->num_nodes + 1, sizeof(int));
        actual_spill = (int *) calloc(graph->num_nodes + 1, sizeof(int));

        RA_simplify(graph, simple_nodes, potential_spill);
        sort_nodes(graph, potential_spill);

        spill = RA_select(graph, simple_nodes, potential_spill, actual_spill);
        count++;
        //register_allocation(program, graph);
    }
    //print_graph(graph);
    //print_IR_tree(program);
    printf("Temps: %d, spills: %d, runs: %d\n", temp_c, spill, count);
    //free(simple_nodes);
    //free(potential_spill);
    //free(actual_spill);
    return graph;
}