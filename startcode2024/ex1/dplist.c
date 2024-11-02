/**
 * \author Jeroen Van Aken, Bert Lagaisse, Ludo Bruynseels
    student: Fulya Albas, student nr: r0913342, academic yr 2024-2025
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "dplist.h"



/*
 * The real definition of struct list / struct node
 */
struct dplist_node {
    dplist_node_t *prev, *next;
    element_t element;
};
typedef void (*free_element_t)(element_t element);
struct dplist {
    dplist_node_t *head;
    dplist_node_t *tail;
    free_element_t free_element;
    // more fields will be added later
};

dplist_t *dpl_create() {
    dplist_t *list;
    list = malloc(sizeof(struct dplist));
    list->head = NULL;
  return list;
}

void dpl_free(dplist_t **list) {

    //TODO: add your code here
    //Do extensive testing with valgrind.
    if (list == NULL || *list == NULL) {
        return;
    }

    dplist_node_t *current = (*list)->head;
    while (current != NULL) {
        dplist_node_t *next = current->next;
        free(current);
        current = next;
    }

    free(*list);
    *list = NULL;
}

/* Important note: to implement any list manipulation operator (insert, append, delete, sort, ...), always be aware of the following cases:
 * 1. empty list ==> avoid errors
 * 2. do operation at the start of the list ==> typically requires some special pointer manipulation
 * 3. do operation at the end of the list ==> typically requires some special pointer manipulation
 * 4. do operation in the middle of the list ==> default case with default pointer manipulation
 * ALWAYS check that you implementation works correctly in all these cases (check this on paper with list representation drawings!)
 **/


dplist_t *dpl_insert_at_index(dplist_t *list, element_t element, int index) {
    dplist_node_t *ref_at_index, *list_node;
    if (list == NULL) return NULL;

    list_node = malloc(sizeof(dplist_node_t));

    list_node->element = element;
    // pointer drawing breakpoint
    if (list->head == NULL) { // covers case 1
        list_node->prev = NULL;
        list_node->next = NULL;
        list->head = list_node;
        // pointer drawing breakpoint
    } else if (index <= 0) { // covers case 2
        list_node->prev = NULL;
        list_node->next = list->head;
        list->head->prev = list_node;
        list->head = list_node;
        // pointer drawing breakpoint
    } else {
        ref_at_index = dpl_get_reference_at_index(list, index);
        assert(ref_at_index != NULL);
        // pointer drawing breakpoint
        if (index < dpl_size(list)) { // covers case 4
            list_node->prev = ref_at_index->prev;
            list_node->next = ref_at_index;
            ref_at_index->prev->next = list_node;
            ref_at_index->prev = list_node;
            // pointer drawing breakpoint
        } else { // covers case 3
            assert(ref_at_index->next == NULL);
            list_node->next = NULL;
            list_node->prev = ref_at_index;
            ref_at_index->next = list_node;
            // pointer drawing breakpoint
        }
    }
    return list;
}

dplist_t *dpl_remove_at_index(dplist_t *list, int index) {

    //TODO: add your code here
    if (list == NULL) {
        return NULL;
    }

    if (list->head == NULL) {
        return NULL;
    }
    dplist_node_t *current = list->head;
    if (index <= 0) {
        if (current->next != NULL) {
            list->head = current->next;
            list->head->prev = NULL;
        } else {
            list->head = NULL;
        }
    } else {
        int i = 0;
        while (current != NULL && i < index) {
            current = current->next;
            i++;
        }
        if (current == NULL) {
            current = list->tail;
        } else {
            if (current->prev != NULL) {
                current->prev->next = current->next;
            }
            if (current->next != NULL) {
                current->next->prev = current->prev;
            }
        }
    }

    dplist_node_t *removed_node = current;
    if (current != NULL) {
        if (list->free_element != NULL) {
            list->free_element(current->element);
        }
        free(current);
    }

    return removed_node;
}

int dpl_size(dplist_t *list) {
    //TODO: add your code here
    if (list == NULL) {
        return -1; // return -1 if the list is NULL
    }
    int size = 0;
    dplist_node_t *current = list->head;
    while (current != NULL) {
        size++;
        current = current->next;
    }
    return size;
}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {
    //int count = 0 ;
    dplist_node_t *dummy = NULL;
    //TODO: add your code here
    if (list == NULL || list->head == NULL) {
        return NULL;
    }
    dplist_node_t *current = list->head;
    if (index <= 0) {
        return current;
    }
    int i = 0;
    while (current->next != NULL && i < index) {
        current = current->next;
        i++;
    }
    return current;
}

element_t dpl_get_element_at_index(dplist_t *list, int index) {

    //TODO: add your code here
    if (list == NULL || list->head == NULL) {
        return NULL;
    }
    dplist_node_t *node = dpl_get_reference_at_index(list, index);
    return node->element;
}

int dpl_get_index_of_element(dplist_t *list, element_t element) {

    //TODO: add your code here
    if (list == NULL) {
        return -1;
    }

    int index = 0;
    dplist_node_t *current = list->head;
    while (current != NULL) {
        if (current->element == element) {
            return index;
        }
        current = current->next;
        index++;
    }

    return -1;}



