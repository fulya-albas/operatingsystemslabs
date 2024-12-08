#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "sbuffer.h"
#include <string.h>
#include <stdbool.h>

#define SENSOR_FILE "sensor_data" //binary data file
#define SENSOR_FILE_EMPTY "sensor_data_empty" //binary data file for testing only
#define OUTPUT_FILE "sensor_data_out.csv"
#define NUM_READERS 2

sbuffer_t *shared_buffer;

pthread_mutex_t file_write_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t eos_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t eos_mutex = PTHREAD_MUTEX_INITIALIZER;
int eos_count = 0; // Count how many reader threads have seen the EOS marker

// Function to write sensor data to CSV file
void write_to_csv(sensor_data_t *data) {
    pthread_mutex_lock(&file_write_mutex);
    FILE *fp = fopen(OUTPUT_FILE, "a");
    if (!fp) {
        perror("Failed to open output CSV file");
        pthread_mutex_unlock(&file_write_mutex);
        return;
    }
    // Write data to CSV in correct format
    fprintf(fp, "%hu,%.4f,%ld\n", data->id, data->value, data->ts);
    fflush(fp);  // Ensure data is written immediately
    fclose(fp);
    pthread_mutex_unlock(&file_write_mutex);
}

// Reads data from file and writes to buffer
void *writer_thread(void *arg) {
    FILE *sens_file = fopen(SENSOR_FILE, "r");
    if (!sens_file) {
        perror("Error opening sensor file");
        exit(EXIT_FAILURE); // Terminate the program
    }

    sensor_data_t record;
    while (fread(&record, sizeof(sensor_data_t), 1, sens_file) == 1) {
        sbuffer_insert(shared_buffer, &record);
        usleep(10000); // Sleep for 10ms
    }

    // Put zero at end of stream for all threads to process
    record.id = 0;
    sbuffer_insert(shared_buffer, &record);

    fclose(sens_file);
    pthread_exit(NULL);
}

// Reads data from the buffer and inserts it into a CSV file via write_to_csv
void *reader_thread(void *arg) {
    sensor_data_t data;

    while (1) {
        sbuffer_remove(shared_buffer, &data);
        if (data.id == 0) { // End-of-stream marker
            pthread_mutex_lock(&eos_mutex);
            eos_count++;
            if (eos_count < NUM_READERS) {
                sbuffer_insert(shared_buffer, &data); // Reinsert for remaining readers
            }
            pthread_mutex_unlock(&eos_mutex);
            break; // Stop reading after processing the end-of-stream marker
        }
        write_to_csv(&data); // Write the data to the CSV
        usleep(25000); // Sleep for 25ms
    }

    pthread_exit(NULL);
}

// Clears the output file before starting
int clear_file(const char *filename) {
    FILE *file = fopen(filename, "w"); // Open in write mode truncates data in file, clearing it
    if (!file) {
        perror("Failed to open file");
        return -1;
    }
    fclose(file); // Close the file
    return 0;
}

int main() {
    static const char *s_out = OUTPUT_FILE;
    clear_file(s_out); // Clear the output file before starting

    // Initialize shared buffer
    printf("Buffer operation initializing\n");
    sbuffer_init(&shared_buffer);

    pthread_t writer, reader1, reader2;
    printf("Buffer operation started, data being sent...\n");

    // Create threads
    pthread_create(&writer, NULL, writer_thread, NULL);
    pthread_create(&reader1, NULL, reader_thread, NULL);
    pthread_create(&reader2, NULL, reader_thread, NULL);

    // Wait for threads to finish
    pthread_join(writer, NULL);
    pthread_join(reader1, NULL);
    pthread_join(reader2, NULL);

    // Clean up
    printf("Buffer operation complete\n");

    sbuffer_free(&shared_buffer);

    return 0;
}
