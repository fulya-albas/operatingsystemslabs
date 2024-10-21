//
// Created by fulya-albas2 on 10/21/24.
//

#include "lab2_sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define frequency 2
#define min_temp -10.0
#define max_temp 35.0

float generate_temp(){
  return ((float)rand()/ RAND_MAX)*(max_temp-min_temp)+min_temp;
  }

void get_current_time(char *buffer, size_t size){
  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);
  strftime(buffer, size,  "%H:%M:%S", tm_info);
  }

int main(){
  srand(time(NULL));
  char time_buffer[20];

  while(1){
    float temperature = generate_temp();
    get_current_time(time_buffer, sizeof(time_buffer));
    printf("Temperature: %1.2f @%s\n", temperature, time_buffer);
    fflush(stdout);

    sleep(frequency);
    }
    return 0;
    }

