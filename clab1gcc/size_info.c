#include <stdio.h>

int main() {
  // Printing sizes of basic data types
  printf("Size of int: %zu bytes\n", sizeof(int));
  printf("Size of float: %zu bytes\n", sizeof(float));
  printf("Size of double: %zu bytes\n", sizeof(double));
  printf("Size of void pointer: %zu bytes\n", sizeof(void*)); // Size of a void pointer (not void)

  // Type qualifiers short and long
  printf("Size of short int: %zu bytes\n", sizeof(short int));
  printf("Size of long int: %zu bytes\n", sizeof(long int));
  printf("Size of long double: %zu bytes\n", sizeof(long double));

  // Checking if char is signed or unsigned
  if ((char) -1 < 0) {
    printf("char is signed on this system\n");
  } else {
    printf("char is unsigned on this system\n");
  }

  return 0;
}


  
