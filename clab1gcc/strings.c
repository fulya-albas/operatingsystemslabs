//
// Created by fulya-albas2 on 10/14/24.
//

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX 100

int is_digit(char c){
  return c>= '0' && c<='9';
  }

int is_upper(char c){
  return c>='A' && c<= 'Z';
  }

int is_lower(char c){
  return c>='a' && c<='z';
  }

 void to_upper(char str[]){
   for(int i=0; str[i] != '\0'; i++){
     if(is_lower(str[i])){
       str[i] = str[i] - 'a' + 'A';
       }
   }
 }

 int main(){
   char first[MAX], second[MAX], name[MAX * 2], str[MAX];
   int year;

   printf("Enter First Name: ");
   scanf("%s", first);
   printf("Enter Second Name: ");
   scanf("%s", second);

   strcpy(str, second);
   to_upper(str);

   if(strcmp(second, str) ==0){
     printf("The second name and it's uppercase version is correct.\n");
    }else{
     printf("The second name and it's lowercase version are different.\n");
     }

   if(strcasecmp(second, str)==0){
     printf("strcasecmp confirms that they are the same, ignoring case.\n");
   }else{
     printf("strcasecmp confirms that they are different, ignoring case.\n");
   }
  strcpy(name, first);
  strcat(name, " ");
  strcat(name, second);
  printf("Full name (using unsafe strcat): %s\n", name);

  printf("Enter your birth year: ");
  scanf("%d", &year);

  snprintf(name, sizeof(name), "%s %s %d", first, second, year);
  printf("Full name with year (using snprintf): %s\n", name);

  char extracted1st[MAX], extracted2nd[MAX];
  int extractedyr;
  sscanf(name, "%s %s %d", extracted1st, extracted2nd, &extractedyr);
  printf("Extracted data - First: %s, Second: %s, Year: %d\n", extracted1st, extracted2nd, extractedyr);

  return 0;
}





