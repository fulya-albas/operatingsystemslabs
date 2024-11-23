#define _GNU_SOURCE

#include "dplist.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef struct {
    int id;
    char* name;
} my_element_t;

void* element_copy(void * element);
void element_free(void ** element);
int element_compare(void * x, void * y);

void * element_copy(void * element) {
    my_element_t* copy = malloc(sizeof (my_element_t));
    char* new_name;
    asprintf(&new_name,"%s",((my_element_t*)element)->name); //asprintf requires _GNU_SOURCE
    assert(copy != NULL);
    copy->id = ((my_element_t*)element)->id;
    copy->name = new_name;
    return (void *) copy;
}

void element_free(void ** element) {
    free((((my_element_t*)*element))->name);
    free(*element);
    *element = NULL;
}

int element_compare(void * x, void * y) {
    return ((((my_element_t*)x)->id < ((my_element_t*)y)->id) ? -1 : (((my_element_t*)x)->id == ((my_element_t*)y)->id) ? 0 : 1);
}

void ck_assert_msg(bool result, char * msg){
    if(!result) printf("%s\n", msg);
}

void yourtest1()
{
    // Test free NULL, don't use callback
    dplist_t *list = NULL;
    dpl_free(&list, false);
    ck_assert_msg(list == NULL, "Failure: expected result to be NULL");

    // Test free NULL, use callback
    list = NULL;
    dpl_free(&list, true);
    ck_assert_msg(list == NULL, "Failure: expected result to be NULL");

    // Test free empty list, don't use callback
    list = dpl_create(element_copy, element_free, element_compare);
    dpl_free(&list, false);
    ck_assert_msg(list == NULL, "Failure: expected result to be NULL");

    // Test free empty list, use callback
    list = dpl_create(element_copy, element_free, element_compare);
    dpl_free(&list, true);
    ck_assert_msg(list == NULL, "Failure: expected result to be NULL");

    // TODO : your test scenarios
    // Test inserting elements
    my_element_t *elem1 = malloc(sizeof(my_element_t));
    elem1->id = 1;
    elem1->name = "Element 1";
    list = dpl_create(element_copy, element_free, element_compare);
    list = dpl_insert_at_index(list, elem1, 0, false);
    ck_assert_msg(dpl_size(list) == 1, "Failure: expected list size to be 1");

    // Test inserting multiple elements
    my_element_t *elem2 = malloc(sizeof(my_element_t));
    elem2->id = 2;
    elem2->name = "Element 2";
    list = dpl_insert_at_index(list, elem2, 1, false);
    ck_assert_msg(dpl_size(list) == 2, "Failure: expected list size to be 2");

    // Test retrieving elements
    my_element_t *retrieved = (my_element_t *)dpl_get_element_at_index(list, 0);
    ck_assert_msg(retrieved != NULL && retrieved->id == 1, "Failure: expected first element to have id 1");

}

int main(void) {

    yourtest1();
    return 0;
}
