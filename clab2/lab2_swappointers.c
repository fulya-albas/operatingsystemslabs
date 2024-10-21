//
// Created by fulya-albas2 on 10/21/24.
//

#include "lab2_swappointers.h"
#include <stdio.h>

void swapPointers(int **p, int **q) {
  void *temp = *p;
  *p = *q;
  *q = temp;
}

int main(void){
  int a = 1;
  int b = 2;

  int *p = &a;
  int *q = &b;

  printf("Before swapPointers: p=%p, q=%p\n", (void*)p, (void*)q);
  swapPointers((void**)&p, (void**)&q);
  printf("After swapPointers: p=%p, q=%p\n", (void*)p, (void*)q);
  return 0;
  }