#include <stdio.h>
int main(){
  printf("size of int: ", sizeof(int), " bytes");
  printf("size of float: ", sizeof(float), " bytes");
  printf("size of double: ", sizeof(double), " bytes");
  printf("size of void*: ", sizeof(void*), " bytes");
  
  printf("Size of short int: %zu bytes\n", sizeof(short int));
  printf("Size of long int: %zu bytes\n", sizeof(long int));
  printf("Size of long double: %zu bytes\n", sizeof(long double));
  
  return 0;
}
  
