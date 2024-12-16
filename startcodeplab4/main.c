#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sbuffer.h"  // Include your buffer header
#include "config.h"  // Include the header where sensor_data_t is defined

// Global variables for synchronization
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t done_cond = PTHREAD_COND_INITIALIZER;
int done_reading = 0;

#include <stdio.h>

void write_to_csv(const sensor_data_t *data) {
    FILE *file = fopen("sensor_data.csv", "a");  // Open in append mode
    if (file == NULL) {
        perror("Failed to open CSV file");
        return;
    }

    // Write the sensor data to the file in CSV format
    fprintf(file, "%hu,%.4f,%ld\n", data->id, data->value, (long)data->ts);

    fclose(file);
}


void *writer_thread(void *arg) {
    sbuffer_t *buffer = (sbuffer_t *)arg;
    FILE *binaryfile = fopen("sensor_data", "rb");
    if (!binaryfile) {
        perror("Failed to open sensor data file");
        return NULL;
    }

    sensor_data_t data_chunk;
    ssize_t items_read;

    while ((items_read = fread(&data_chunk.id, sizeof(sensor_id_t), 1, binaryfile)) > 0) {
        if (fread(&data_chunk.value, sizeof(sensor_value_t), 1, binaryfile) <= 0) {
            break;
        }

        if (fread(&data_chunk.ts, sizeof(sensor_ts_t), 1, binaryfile) <= 0) {
            break;
        }

        sbuffer_insert(buffer, &data_chunk);
        usleep(10000); // Delay to simulate real-time data feeding
    }

    // Insert end-of-stream marker after all data has been processed
    sensor_data_t end_marker = {0};  // id = 0 indicates end of stream
    sbuffer_insert(buffer, &end_marker);
    fclose(binaryfile);
    return NULL;
}

void *reader_thread(void *arg) {
    sbuffer_t *buffer = (sbuffer_t *)arg;
    sensor_data_t data;

    while (1) {
        int result = sbuffer_remove(buffer, &data);

        if (result == SBUFFER_NO_DATA) {
            usleep(5000); // Add delay if buffer is empty
            continue;  // Continue if no data
        }

        if (data.id == 0) {  // End-of-stream marker
            pthread_mutex_lock(&mutex);
            done_reading++;
            if (done_reading == 2) {
                pthread_cond_signal(&done_cond);  // Signal main thread to finish
            }
            pthread_mutex_unlock(&mutex);
            break; // Exit the loop
        }

        // Regular data processing
        printf("Reading data: ID = %hu, Value = %.4f, Timestamp = %ld\n", data.id, data.value, (long)data.ts);
        write_to_csv(&data); // Assuming you have a function to write data to CSV
        usleep(25000);  // 25 milliseconds delay for processing
    }
    return NULL;
}

int main() {
    sbuffer_t *buffer;
    pthread_t writer, reader1, reader2;

    printf("Starting read&write buffer process\n");

    // Initialize the buffer
    if (sbuffer_init(&buffer) != SBUFFER_SUCCESS) {
        perror("Failed to initialize buffer");
        return 1;
    }

    // Create writer and reader threads
    if (pthread_create(&writer, NULL, writer_thread, buffer) != 0) {
        perror("Failed to create writer thread");
        return 1;
    }

    if (pthread_create(&reader1, NULL, reader_thread, buffer) != 0) {
        perror("Failed to create reader thread 1");
        return 1;
    }

    if (pthread_create(&reader2, NULL, reader_thread, buffer) != 0) {
        perror("Failed to create reader thread 2");
        return 1;
    }

    // Wait for both reader threads to finish
    pthread_mutex_lock(&mutex);
    while (done_reading < 2) {
        pthread_cond_wait(&done_cond, &mutex);  // Wait until both readers are done
    }
    pthread_mutex_unlock(&mutex);

    // Wait for the threads to finish
    pthread_join(writer, NULL);
    pthread_join(reader1, NULL);
    pthread_join(reader2, NULL);

    // Clean up and free the buffer
    sbuffer_free(&buffer);

    printf("Finished buffer process\n");

    return 0;
}
