#ifndef REGISTER_ALLOCATION_H
#define REGISTER_ALLOCATION_H

#include "ir.h"

typedef struct RA_graph RA_graph;
typedef struct RA_node RA_node;
typedef enum reg_color reg_color;

/*
 * A struct that represents a graph of temporary variables.
 */
struct RA_graph {
    int num_nodes;
    int **adj_matrix;
    RA_node **nodes;
};

/* A struct that represents a node in the register allocation graph 
 * Color holds the allocated register of a temporary
 * The connections array holds the indices of the temporaries that are alive at the same time
 */
struct RA_node {
    int color;
    int num_edges;
    int *connections;
    IR_operation *definition;
};

// Constant for max registers available during register allocation
#define MAX_REG 8

enum reg_color {
    R15 = 1,
    R14,
    R13,
    R12,
    R11,
    R10,
    R9,
    R8,
    RBX,
    RCX,
    //RAX,
    //RDX,
    // needs precoloring to work with rax and rdx cause div and mod
};

RA_graph *create_graph(int);
RA_node *create_graph_node(int);
void connect_nodes(RA_graph *, int, int);
void connect_graph(RA_graph *, frame *);
void print_graph(RA_graph *);
void print_adj_matrix(RA_graph *);
RA_graph *register_allocation(int, frame *);
void kill_graph(RA_graph *graph);



#endif