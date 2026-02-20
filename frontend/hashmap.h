#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define PRIME 257

typedef struct table_entry table_entry;
typedef struct table_entry** buckets;
typedef struct hash_table hash_map;

struct table_entry {
    char *key;
    void *value;
    table_entry *next;
};

struct hash_table {
    int size, capacity;
    buckets buckets;
};

/*
 * Initializes a new table entry with the provided key and value.
 */
table_entry *create_entry(const char *, void *);

/*
 * Adds a new table_entry to the tail of the given table_entry.
 */
void add_entry(table_entry *, table_entry *);

/*
 * Checks if the key already exists in the given entries
 * Returns 1 if the key is found, 0 otherwise.
 */
int contains_key(table_entry *, const char *);

/*
 * Check if key exists in the given hashmap
 * Returns 1 if the key is found, 0 otherwise.
 */
int *hash_map_contains(hash_map *, const char *);

/*
 * Recursively destroys a table_entry and all entries that it might point to.
 */
void destroy_entry(table_entry *);

/*
 * Initializes an empty hash map with the given capacity
 */
hash_map *create_hash_map(int);

/*
 * Inserts the key-value pair into the hash map.
 * Returns 0 upon succesful insertion, -1 otherwise (Maybe more descriptive errors needed?)
 */
int hash_map_insert(hash_map *, const char *, void *);

/*
 * Deletes the entry associated with the key from the hash map.
 * Returns 0 upon succesful deletion, -1 otherwise.
 */
int hash_map_delete(hash_map *, const char *);

/*
 * Finds and gets the content associated with the given key in the hash map
 * Returns NULL if the given key is not in the table.
 */
void *hash_map_get(hash_map *, const char *);

/*
 * Returns the index of the provided key in hash map
 */
int hash_function(hash_map *, const char *);

/*
 * Destroys the hash map and all its entries
 */
void destroy_hash_map(hash_map *);

#endif