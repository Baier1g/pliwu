#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "hashmap.h"

typedef struct symbol_table symbol_table;

struct symbol_table {
    hash_map *map;
    symbol_table *outer, *global;
};

/*
 * Creates a new symbol table from an outer and global table
 */
symbol_table *create_symbol_table(symbol_table *, symbol_table *);
void destroy_symbol_table(symbol_table *);
/*
 * Inserts the key-value pair into the symbol table.
 * Returns 0 upon succesful insertion, -1 otherwise
 */
int symbol_table_insert(symbol_table *, char *, void *);
void *symbol_table_get(symbol_table *, char *);
/*
 * recursively checks if the key already exists in the given symbol table
 * Returns 1 if the key is found, 0 otherwise.
 */
int symbol_table_contains(symbol_table *, char *);

#endif