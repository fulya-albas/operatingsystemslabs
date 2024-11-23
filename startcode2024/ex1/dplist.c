/**
 * \author Jeroen Van Aken, Bert Lagaisse, Ludo Bruynseels
    student: Fulya Albas, student nr: r0913342, academic yr 2024-2025
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "dplist.h"


/*
 * The real definition of struct list / struct node
 */
struct dplist_node {
    dplist_node_t *prev, *next;
    element_t element;
};

struct dplist {
    dplist_node_t *head;
};

// Helper function to copy an element (string)
element_t element_copy(const element_t element) {
    if (element == NULL) return NULL;
    char* new_str = malloc(strlen(element) + 1); // +1 for the null terminator
    if (new_str == NULL) {
        return NULL;
    }
    strcpy(new_str, element);
    return new_str;
}

// Helper function to free an element (string)
void element_free(element_t element) {
    free(element);
}

dplist_t *dpl_create() {
    dplist_t *list = malloc(sizeof(dplist_t));
    assert(list != NULL);
    list->head = NULL;
    return list;
}

void dpl_free(dplist_t **list) {
    if (list == NULL || *list == NULL) return;

    dplist_node_t *current = (*list)->head;
    while (current != NULL) {
        dplist_node_t *next = current->next;
        element_free(current->element);
        free(current);
        current = next;
    }

    free(*list);
    *list = NULL;
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

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {
    if (list == NULL || list->head == NULL) return NULL;

    dplist_node_t *current = list->head;
    if (index <= 0) return current;

    int i = 0;
    while (current->next != NULL && i < index) {
        current = current->next;
        i++;
    }
    return current;
}

element_t dpl_get_element_at_index(dplist_t *list, int index) {
    if (list == NULL || list->head == NULL) return NULL;

    dplist_node_t *node = dpl_get_reference_at_index(list, index);
    return node->element;
}

int dpl_get_index_of_element(dplist_t *list, element_t element) {
    if (list == NULL) {
        return -1;
    }

    int index = 0;
    dplist_node_t *current = list->head;
    while (current != NULL) {
        if (strcmp(current->element, element) == 0) { // Use strcmp for string comparison
            return index;
        }
        current = current->next;
        index++;
    }

    return -1;
}


dplist_t *dpl_insert_at_index(dplist_t *list, element_t element, int index) {
    if (list == NULL) return NULL;

    dplist_node_t *new_node = malloc(sizeof(dplist_node_t));
    assert(new_node != NULL);

    new_node->element = element_copy(element);

    if (list->head == NULL) {
        new_node->prev = NULL;
        new_node->next = NULL;
        list->head = new_node;
    } else if (index <= 0) {
        new_node->prev = NULL;
        new_node->next = list->head;
        list->head->prev = new_node;
        list->head = new_node;
    } else {
        dplist_node_t *ref_at_index = dpl_get_reference_at_index(list, index);
        if (index < dpl_size(list)) {
            new_node->prev = ref_at_index->prev;
            new_node->next = ref_at_index;
            if (ref_at_index->prev != NULL) {
                ref_at_index->prev->next = new_node;
            }
            ref_at_index->prev = new_node;
        } else {
            new_node->next = NULL;
            new_node->prev = ref_at_index;
            ref_at_index->next = new_node;
        }
    }
    return list;
}

dplist_t *dpl_remove_at_index(dplist_t *list, int index) {
    if (list == NULL || list->head == NULL) return list;

    dplist_node_t *current = list->head;
    if (index <= 0) {
        list->head = current->next;
        if (list->head != NULL) list->head->prev = NULL;
    } else {
        current = dpl_get_reference_at_index(list, index);
        if (current->prev != NULL) {
            current->prev->next = current->next;
        }
        if (current->next != NULL) {
            current->next->prev = current->prev;
        }
    }

    element_free(current->element);
    free(current);
    return list;
}