#include "linked_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

linked_list *linked_list_new() {
    struct linked_list *ptr;
    ptr = (struct linked_list*) malloc(sizeof(struct linked_list));
    if (!ptr) {
        return NULL;
    }
    ptr->head = NULL;
    ptr->tail = NULL;
    ptr->size = 0;
    return ptr;
}

void linked_list_delete(linked_list *ll) {
    linked_list_node *nextptr, *node;
    int downsize = ll->size;
    nextptr = node = ll->head;
    while (downsize != 0) {
        downsize--;
        nextptr = node->next;
        free(node);
        node = nextptr;
    }
    free(ll);
}

linked_list_node *linked_list_append(linked_list *ll, void *elem) {
    linked_list_node *ptr = (struct linked_list_node *)malloc(sizeof(struct linked_list_node));
    if (!ptr) {
        return NULL;
    }
    ptr->data = elem;
    ptr->next = NULL;
    if (ll->tail) {
        ll->tail->next = ptr;
        ptr->prev = ll->tail;
    } else {
        ptr->prev = NULL;
        ll->head = ptr;
    }
    ll->tail = ptr;
    ll->size++;
    return ptr;
}

void *linked_list_pop_front(linked_list *ll) {
    assert(ll->size != 0);
    linked_list_node *node = ll->head;
    if (ll->size == 1) {
        ll->head = NULL;
        ll->tail = NULL;
        ll->size = 0;
        return node->data;
    }
    node->next->prev = NULL;
    ll->head = node->next;
    ll->size--;
    return node->data;
}

linked_list_node *linked_list_find(linked_list *ll, void *data) {
    linked_list_node *node = ll->head;
    while (node) {
        if (data == node->data) {
            return node;
        } else {
            node = node->next;
        }
    }
    return NULL;
}

void *linked_list_remove(linked_list *ll, linked_list_node *node) {
    if (!linked_list_find(ll, node->data)) {
        return NULL;
    }
    void *dataptr = NULL;
    if (ll->head == node) {
        dataptr = linked_list_pop_front(ll);
    } else if (ll->tail == node) {
        ll->tail = node->prev;
        ll->tail->next = NULL;
        dataptr = node->data;
        ll->size--;
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        dataptr = node->data;
        ll->size--;
    }
    free(node);
    return dataptr;
}
