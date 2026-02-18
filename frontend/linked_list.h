#ifndef LINKED_LIST_H
#define LINKED_LIST_H

// A linked list containing any type of pointer.
// The linked list does _not_ own its elements.

typedef struct linked_list linked_list;
typedef struct linked_list_node linked_list_node;

struct linked_list
{
    linked_list_node *head;
    linked_list_node *tail;
    int size;
};

struct linked_list_node
{
    linked_list_node *next;
    linked_list_node *prev;
    void *data;
};

// Allocate and initialize an empty linked list.
// Returns: a pointer to the new linked list, or NULL on error.
// Post: the caller owns the linked list.
linked_list *linked_list_new();

// Deallocate the given linked list, including all nodes
// (but _not_ the data they point to, the user owns that).
void linked_list_delete(linked_list *ll);

// Append a the given element to the list.
// The linked list does _not_ take ownership over the element
// (only the linked list node).
// Returns: a pointer to the node with the new element, or NULL on error.
linked_list_node *linked_list_append(linked_list *ll, void *elem);

// Remove and return the first element from the given list.
// Pre: ll->size != 0
void *linked_list_pop_front(linked_list *ll);

// Find the linked list node containing the given element.
// Returns: a pointer to the found node, or NULL if the element was not found.
linked_list_node *linked_list_find(linked_list *ll, void *elem);

// Remove the given node from the given linked list (and deallocate it).
// Pre: node must belong to ll
// Returns: node->data
void *linked_list_remove(linked_list *ll, linked_list_node *node);

#endif // linked_list_H