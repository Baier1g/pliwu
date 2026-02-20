#include "symbol_table.h"

symbol_table *create_symbol_table(symbol_table *outer, symbol_table *global) {
    symbol_table *table = malloc(sizeof(symbol_table));
    table->map = create_hash_map(128);
    table->global = global;
    table->outer = outer;
    return table;
}

void destroy_symbol_table(symbol_table *table) {
    destroy_hash_map(table->map);
    free(table);
}

int symbol_table_insert(symbol_table *table, char *key, void *value) {
    if (symbol_table_contains(table->global, key)) {
        return -1;
    }
    return hash_map_insert(table->map, key, value);
}

void *symbol_table_get(symbol_table *table, char* key) {
    if (!table) {
        return NULL;
    }
    void *tmp;
    if (!(tmp = hash_map_get(table->map, key))) {
        tmp = symbol_table_get(table->outer, key);
    }
    return tmp;
}

