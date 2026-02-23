#include "hashmap.h"

table_entry *create_entry(const char *key, void *value) {
    table_entry *node = (table_entry*) calloc(1, sizeof(table_entry));
    if (!node) {
        printf("died");
        return NULL;
    }
    node->key = (char*) calloc((strlen(key) + 1), sizeof(char));
    if (!node->key) {
        printf("died string");
        return NULL;
    }
    strncpy(node->key, key, strlen(key));
    node->value = value;
    node->next = NULL;
    return node;
}

void add_entry(table_entry* old_entry, table_entry* new_entry) {
    table_entry* end = old_entry;
    while (end->next) {
        end = end->next;
    }
    end->next = new_entry;
}

int contains_key(table_entry *node, const char *key) {
    table_entry *tmp = node;
    while (tmp) {
        if (!strcmp(tmp->key, key)) {
            return 1;
        }
        tmp = tmp->next;
    }
    return 0;
} 

void destroy_entry(table_entry *entry) {
    if (!entry) {
        return;
    }
    destroy_entry(entry->next);
    free(entry->key);
    free(entry);
}

hash_map *create_hash_map(int capacity) {
    hash_map *map = malloc(sizeof(hash_map));
    //assert(map);
    map->size = 0;
    map->capacity = capacity;
    map->buckets = calloc(capacity, sizeof(table_entry*));
    //assert(map->buckets);
    return map;
}

int hash_map_insert(hash_map *map, const char *key, void *value) {
    int index = hash_function(map, key);
    table_entry *node = map->buckets[index];
    if (node) {
        if (!contains_key(node, key)) {
            add_entry(node, create_entry(key, value));
            map->size++;
        } else {
            return -1;
        }
    } else {
        map->buckets[index] = create_entry(key, value);
        map->size++;
    }
    return 0;
}

int hash_map_delete(hash_map *map, const char *key) {
    int index = hash_function(map, key);
    table_entry *node = map->buckets[index];
    if (node) {
        table_entry *curr, *prev = NULL;
        curr = node;
        while (curr) {
            if (!strcmp(curr->key, key)) {
                if (!prev) {
                    prev = curr->next;
                    curr->next = NULL;
                    destroy_entry(curr);
                    map->buckets[index] = prev;
                    map->size--;
                } else {
                    prev->next = curr->next;
                    curr->next == NULL;
                    destroy_entry(curr);
                    map->size--;
                }
                return 0;
            }
            prev = curr;
            curr = curr->next;
        }
    }
    return -1;
}

void *hash_map_get(hash_map *map, const char *key) {
    int index = hash_function(map, key);
    table_entry *node = map->buckets[index];
    while (node) {
        if (!strcmp(node->key, key)) {
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}

int hash_function(hash_map *map, const char *key) {
    int sum = 0, factor = PRIME;
    for (int i = 0; i < strlen(key); i++) {
        sum = ((sum % map->capacity) + (((int) key[i]) * factor) % map->capacity) % map->capacity;
        factor = ((factor % __INT16_MAX__) * (PRIME % __INT16_MAX__)) % __INT16_MAX__;
    }
    return sum;
}

void destroy_hash_map(hash_map *map) {
    for (int i = 0; i < map->capacity; i++) {
        if (map->buckets[i]) {
            destroy_entry(map->buckets[i]);
        }
    }
    free(map->buckets);
    free(map);
}

int hash_map_contains(hash_map *map, const char *key) {
    int index = hash_function(map, key);
    table_entry *node = map->buckets[index];
    while (node) {
        if (!strcmp(node->key, key)) {
            return 1;
        }
        node = node->next;
    }
    return 0;
}
/*
int main() {
    hash_map *map = create_hash_map(128);
    if (!map) {
        printf("fucked\n");
        return -1;
    }
    hash_map_insert(map, "henlo", (void *) 5);
    hash_map_insert(map, "fren", (void *) 4);
    hash_map_insert(map, "gg",  (void * ) 6);
    hash_map_insert(map, "frend", (void *) 10);
    hash_map_insert(map, "frenzy", (void *) 4);
    if (hash_map_insert(map, "frenzy", (void *) 20)) {
        printf("Shit's already there, fam\n");
    }
    printf("Indices: %d, %d, %d, %d, %d\n", hash_function(map, "henlo"), hash_function(map, "fren"), hash_function(map, "gg"), hash_function(map, "frend"), hash_function(map, "frenzy"));
    if (!hash_map_delete(map, "henlo")) {
        printf("Deletion succesful\n");
    }
    if (!hash_map_get(map, "henlo")) {
        printf("Truly gone :bless:\n");
    }
    printf("Indices: %d, %d, %d, %d\n", hash_function(map, "fren"), hash_function(map, "gg"), hash_function(map, "frend"), hash_function(map, "frenzy"));
    printf("Key: %s, value: %d\n", "gg", (int) hash_map_get(map, "gg"));

    destroy_hash_map(map);
    return 0;
}
*/