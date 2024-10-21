#include <stdio.h>
#include <stdlib.h> // for malloc and free

typedef struct {
    short day, month;
    unsigned year;
} date_t;

date_t * date_struct(int day, int month, int year) {
    date_t *dummy = (date_t *)malloc(sizeof(date_t));  // Allocate on the heap
    if (dummy == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);  // Handle memory allocation failure
    }
    dummy->day = (short)day;
    dummy->month = (short)month;
    dummy->year = (unsigned)year;
    return dummy;
}

int main(void) {
    int day, month, year;
    date_t *d;

    printf("\nGive day, month, year: ");
    scanf("%d %d %d", &day, &month, &year);

    d = date_struct(day, month, year);

    // Print the date
    printf("\ndate struct values: %d-%d-%d\n", d->day, d->month, d->year);

    // Free allocated memory
    free(d);

    return 0;
}
