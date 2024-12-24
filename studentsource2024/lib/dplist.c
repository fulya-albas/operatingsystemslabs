#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "dplist.h"

struct dplist_node {
    struct dplist_node *prev, *next;
    void *element;
};

struct dplist {
    struct dplist_node *head;
    void *(*element_copy)(void *src_element);
    void (*element_free)(void **element);
    int (*element_compare)(void *x, void *y);
};

dplist_t *dpl_create(
        void *(*element_copy)(void *src_element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y)
) {
    dplist_t *list = malloc(sizeof(struct dplist));
    if (list == NULL) return NULL;
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}

void dpl_free(dplist_t **list, bool free_element) {
    if (list == NULL || *list == NULL) return;
    dplist_node_t *current = (*list)->head;
    dplist_node_t *next_node;
    while (current != NULL) {
        next_node = current->next;
        if (free_element && (*list)->element_free) {
            (*list)->element_free(&current->element);
        }
        free(current);
        current = next_node;
    }
    free(*list);
    *list = NULL;
}

dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) {
    if (list == NULL) return NULL;
    //allocating memory for new node
    dplist_node_t *new_node = malloc(sizeof(struct dplist_node));
    if (new_node == NULL) return NULL; // return NULL if memory allocation fails

    new_node->element = (insert_copy && list->element_copy) ? list->element_copy(element) : element;
    new_node->next = NULL;
    new_node->prev = NULL;

    if (list->head == NULL || index <= 0) {
        new_node->next = list->head;
        if (list->head != NULL) {
            list->head->prev = new_node;
        }
        list->head = new_node; // set new node as the head
    } else {
        dplist_node_t *current = list->head;
        int current_index = 0;
        while (current_index < index - 1 && current->next != NULL) {
            current = current->next;
            current_index++;
        }
        if (current->next == NULL) {
            current->next = new_node; // link the new node at the end
            new_node->prev = current; // set new node's prev to current
        } else {
            new_node->next = current->next; // set next of new node to current's next
            new_node->prev = current;
            current->next->prev = new_node;
            current->next = new_node;
        }
    }
    return list;
}

dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) {
    if (list == NULL || list->head == NULL) return list;
    dplist_node_t *current = list->head;
    if (index <= 0) {
        list->head = current->next;
        if (list->head != NULL) {
            list->head->prev = NULL;
        }
        if (free_element && list->element_free) {
            list->element_free(&current->element);
        }
        free(current); // free the current node
        return list;
    }
    for (int i = 0; current != NULL && i < index; i++) {
        current = current->next;
    }
    if (current == NULL) {
        current = list->head;
        while (current->next != NULL) {
            current = current->next;
        }
    }
    if (current != NULL) {
        if (free_element && list->element_free) {
            list->element_free(&current->element);
        }
        if (current->prev != NULL) {
            current->prev->next = current->next; // link previous to next
        }
        if (current->next != NULL) {
            current->next->prev = current->prev; // link next to previous
        }
        if (current == list->head) {
            list->head = current->next;
        }
        free(current); // free the current node
    }
    return list;
}

int dpl_size(dplist_t *list) {
    if (list == NULL) return -1;
    int size = 0;
    dplist_node_t *current = list->head;
    while (current != NULL) {
        size++;
        current = current->next;
    }
    return size;
}

void *dpl_get_element_at_index(dplist_t *list, int index) {
    if (list == NULL || index < 0) {
        return NULL;
    }
    dplist_node_t *current = list->head;
    for (int i = 0; current != NULL && i < index; i++) {
        current = current->next; // to the next node
    }

    // If current is NULL, return the last node's element
    if (current == NULL) {
        current = list->head;
        while (current != NULL && current->next != NULL) {
            current = current->next; //to the last node
        }
    }

    // return the element at the found node or null if the list was empty
    return (current != NULL) ? current->element : NULL;
}


int dpl_get_index_of_element(dplist_t *list, void *element) {
    // check if the list is NULL
    if (list == NULL) {
        return -1;
    }
    dplist_node_t *current = list->head;
    int index = 0;

    while (current != NULL) {
        if (list->element_compare(current->element, element) == 0) {
            return index;
        }
        current = current->next;
        index++;
    }
    return -1; //if the element was not found
}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {
    if (list == NULL || list->head == NULL) {
        return NULL;
    }
    dplist_node_t *current = list->head;
    if (index <= 0) {
        return current; // return the head node
    }
    //to the specified index
    for (int i = 0; current != NULL && i < index; i++) {
        current = current->next; //to the next node
    }
    // if current is null; return the last node
    if (current == NULL) {
        //to the last node
        current = list->head;
        while (current->next != NULL) {
            current = current->next; // move to the last node
        }
    }
    return current;
}

void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) {
    if (list == NULL || reference == NULL || list->head == NULL) {
        return NULL;
    }

    dplist_node_t *current = list->head;

    while (current != NULL) {
        if (current == reference) {
            return current->element;
        }
        current = current->next;
    }
    return NULL;
}
