/**
* author: Fulya Albas, student nr: r0913342, academic yr: 2024-2025
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <memory.h>
#include <unistd.h>
#include <pthread.h>
#include "sensor_db.h"
#include "sbuffer.h"
#include "datamgr.h"
#include "lib/dplist.h"
#include "config.h"
#include <stdbool.h>

#define SENSOR_MAP "room_sensor.map"

static dplist_t *sensor_list;



struct element {
  sensor_id_t sensor_id;
  int room_id; //we add this to store the roomid from the sensor map in the element
  sensor_value_t queue[RUN_AVG_LENGTH];
  sensor_ts_t last_modified;
};

struct sensor_mapping {
  int room_id;
  int sensor_id;
} sensormapids;

void* element_copy(void* element);
void element_free(void** element);
int element_compare(void* x, void* y);

// Initialize the data manager
void datamgr_init() {
    FILE *fp_sensor_map = fopen(SENSOR_MAP, "r");
    ERROR_HANDLER(!fp_sensor_map, "can't read map file");
    sensor_list = dpl_create(element_copy, element_free, element_compare);

    while(1){
      fscanf(fp_sensor_map, "%i %i", &sensormapids.room_id, &sensormapids.sensor_id);
      if(feof(fp_sensor_map)){break;}
      element_t *sensor = malloc(sizeof(element_t));

      sensor->sensor_id = sensormapids.sensor_id;
      sensor->room_id = sensormapids.room_id;
      sensor->last_modified = 0;
      for(int i=0; i<RUN_AVG_LENGTH; i++){
        sensor->queue[i] = ((float)(SET_MIN_TEMP + SET_MAX_TEMP))/2.0;

      }
      dpl_insert_at_index(sensor_list, sensor, 0, true);
    }
    printf("starting data manager!");

    sbuffer_node_t *node = NULL;
    sensor_data_t *data;
    do{
      do{
        int result = sbuffer_read(&node, &data);
        if (result != SBUFFER_NO_DATA) break;
        usleep(1);
      } while(1);

      if (data->id==0) break; //we do not want to carry on if the end of stream marker is in the buffer
      //onto checking if the sensor is in our list
      element_t *sensorofinterest;
      int found = false;
      for(int i=0; i<dpl_size(sensor_list); i++){
        sensorofinterest = (element_t *) dpl_get_element_at_index(sensor_list, i);
        if(sensorofinterest->sensor_id == data->id){
          found = true;
          break;
        }
      }

      //onto inserting the data into the array (queue)
      if(found){
        printf("latest data read: %i %f %li", data->id, data->value, data->ts);
        for (int i=RUN_AVG_LENGTH-1; i>0; --i){ //we loop backwards through the queue
          sensorofinterest->queue[i] = sensorofinterest->queue[i-1];
        }
        sensorofinterest->queue[0] = data->value;
        sensorofinterest->last_modified = data->ts;
        //onto checking the average, considering the latest data entry
        sensor_value_t curr_avg = datamgr_get_avg(sensorofinterest->queue);
          if (curr_avg > SET_MAX_TEMP) {
              printf("warning: sensor %i detected temp too hot %f > %d", data->id, curr_avg, SET_MAX_TEMP);
              write_to_log(LOG_TOO_HOT, data->id, curr_avg);
          } else if (curr_avg < SET_MIN_TEMP) {
              printf("warning: sensor %i detected temp too cold %f < %d", data->id, curr_avg, SET_MIN_TEMP);
              write_to_log(LOG_TOO_COLD, data->id, curr_avg);
          }
      } else {
          // case: invalid sensor id
          printf("warning: sensor %i not found in map", data->id);
          write_to_log(LOG_INVALID_ID, data->id, 0);
      }
    } while (1);
    datamgr_free();
    pthread_exit(NULL);
}



// Process a sensor reading
void datamgr_process_reading(sensor_id_t sensor_id, double value, time_t timestamp) {
    sensor_data_t temp_sensor;
    temp_sensor.id = sensor_id;  // Use 'id' from the original sensor_data_t
    int index = dpl_get_index_of_element(sensor_list, &temp_sensor);
    if (index == -1) {
        fprintf(stderr, "Sensor ID %u not found\n", sensor_id);
        return;
    }

    // Retrieve the sensor data
    sensor_data_t* sensor = (sensor_data_t*)dpl_get_element_at_index(sensor_list, index);

    // Update the sensor reading value and timestamp
    sensor->value = value;
    sensor->ts = timestamp;

    // Log the updated sensor data (optional)
    printf("Updated Sensor ID: %u, Value: %.2f, Timestamp: %ld\n", sensor->id, sensor->value, (long)sensor->ts);
}

// Free the data manager
void datamgr_free() {
    dpl_free(&sensor_list, true);
}

// Retrieve room ID for a given sensor (you may need to adjust if you have different data)
uint16_t datamgr_get_room_id(sensor_id_t sensor_id) {
    sensor_data_t temp_sensor;
    temp_sensor.id = sensor_id;
    int index = dpl_get_index_of_element(sensor_list, &temp_sensor);
    if (index == -1) {
        return 0;  // Sensor not found
    }
    sensor_data_t* sensor = (sensor_data_t*)dpl_get_element_at_index(sensor_list, index);
    return sensor->id;  // Adjust to match your desired logic for room_id
}

// Retrieve the running average for a given sensor (if you are calculating it)
static sensor_value_t datamgr_get_avg(sensor_value_t const queue[RUN_AVG_LENGTH]) {
    //sensor_data_t temp_sensor;
    //temp_sensor.id = sensor_id;
    //int index = dpl_get_index_of_element(sensor_list, &temp_sensor);
    //if (index == -1) {
    //    return 0.0;  // Sensor not found
    //}
    //sensor_data_t* sensor = (sensor_data_t*)dpl_get_element_at_index(sensor_list, index);
    //return sensor->value; // Adjust based on your running average calculation
    float sum = 0;
    for (int num = 0; num<RUN_AVG_LENGTH; ++num) {
      sum +=queue[num];
    }
    return sum/(float) RUN_AVG_LENGTH; //type casting to float, to avoid trucation of decimals
}

// Callback functions for dplist
void* element_copy(void* element) {
    sensor_data_t* new_sensor = malloc(sizeof(sensor_data_t));
    assert(new_sensor != NULL);
    memcpy(new_sensor, element, sizeof(sensor_data_t));
    return new_sensor;
}

void element_free(void** element) {
    free(*element);
    *element = NULL;
}

int element_compare(void* x, void* y) {
    sensor_data_t* sensor_x = (sensor_data_t*)x;
    sensor_data_t* sensor_y = (sensor_data_t*)y;
    return (sensor_x->id - sensor_y->id);
}

// Parsing sensor map and data files
void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data) {
    assert(fp_sensor_map != NULL);
    assert(fp_sensor_data != NULL);

    uint16_t room_id;
    sensor_id_t sensor_id;
    double sensor_value;
    sensor_ts_t sensor_timestamp;

    // Declare the dynamic list to store sensor data
    dplist_t *sensor_list = dpl_create(element_copy, element_free, element_compare);

    // First, parse the sensor map file to get sensor ids and room ids
    while (fscanf(fp_sensor_map, "%hu %hu", &room_id, &sensor_id) == 2) {
        // Allocate memory for a new sensor_data_t struct
        sensor_data_t *sensor = malloc(sizeof(sensor_data_t));
        if (sensor == NULL) {
            perror("Memory allocation failed for sensor");
            fclose(fp_sensor_map);
            fclose(fp_sensor_data);
            return;
        }

        // Initialize the sensor data
        sensor->id = sensor_id;
        sensor->value = 0.0;
        sensor->ts = 0;  // Default timestamp (could be updated later)

        // Insert the sensor data into the list
        sensor_list = dpl_insert_at_index(sensor_list, sensor, dpl_size(sensor_list), true);
    }

    // Now, parse the sensor data binary file
    while (fread(&sensor_id, sizeof(sensor_id_t), 1, fp_sensor_data) == 1) {
        // Read the value and timestamp for the sensor
        if (fread(&sensor_value, sizeof(sensor_value_t), 1, fp_sensor_data) != 1 ||
            fread(&sensor_timestamp, sizeof(sensor_ts_t), 1, fp_sensor_data) != 1) {
            fprintf(stderr, "Error reading sensor data for sensor ID: %u\n", sensor_id);
            continue;  // Skip to the next sensor
        }

        // Find the sensor in the list based on its sensor_id
        sensor_data_t temp_sensor;
        temp_sensor.id = sensor_id;
        int index = dpl_get_index_of_element(sensor_list, &temp_sensor);

        if (index == -1) {
            fprintf(stderr, "Sensor ID %u not found in map file\n", sensor_id);
            continue;  // Skip this sensor if not found
        }

        // Retrieve the actual sensor data object
        sensor_data_t *sensor = (sensor_data_t*)dpl_get_element_at_index(sensor_list, index);

        // Update the sensor value and timestamp
        sensor->value = sensor_value;
        sensor->ts = sensor_timestamp;

        // Log the updated sensor data (optional)
        printf("Sensor ID: %u, Value: %.2f, Timestamp: %ld\n",
                sensor->id, sensor->value, (long)sensor->ts);
    }

    // Close the files
    fclose(fp_sensor_map);
    fclose(fp_sensor_data);
}
