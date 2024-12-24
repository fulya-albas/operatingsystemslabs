#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "connmgr.h"
#include "sensor_db.h"
#include "sbuffer.h"
#include "config.h"
#include <inttypes.h> // Include this for PRIu16


#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

sbuffer_t *buffer;

void *sensor_data_receiver(void *arg) {
    tcpsock_t *client;
    sensor_data_t data;
    int bytes;

    while (1) {
        if (tcp_active_open(&client, SERVER_PORT, SERVER_IP) != TCP_NO_ERROR) {
            fprintf(stderr, "Error establishing TCP connection with sensor node.\n");
            continue;
        }

        // Receive data
        bytes = sizeof(data.id);
        if (tcp_receive(client, (void *) &data.id, &bytes) != TCP_NO_ERROR) {
            fprintf(stderr, "Error receiving sensor ID.\n");
            continue;
        }

        bytes = sizeof(data.value);
        if (tcp_receive(client, (void *) &data.value, &bytes) != TCP_NO_ERROR) {
            fprintf(stderr, "Error receiving sensor value.\n");
            continue;
        }

        bytes = sizeof(data.ts);
        if (tcp_receive(client, (void *) &data.ts, &bytes) != TCP_NO_ERROR) {
            fprintf(stderr, "Error receiving timestamp.\n");
            continue;
        }

        // Insert the received data into the buffer
        sbuffer_insert(buffer, &data);
        printf("Sensor data received: ID=%" PRIu16 ", Value=%.2f, Timestamp=%ld\n", data.id, data.value, (long)data.ts);


        // Close the client connection
        if (tcp_close(&client) != TCP_NO_ERROR) {
            fprintf(stderr, "Error closing TCP connection.\n");
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t receiver_thread;

    // Initialize the shared buffer
    if (sbuffer_init(&buffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "Failed to initialize buffer.\n");
        return EXIT_FAILURE;
    }

    // Start the sensor data receiver thread
    if (pthread_create(&receiver_thread, NULL, sensor_data_receiver, NULL) != 0) {
        fprintf(stderr, "Failed to create sensor data receiver thread.\n");
        return EXIT_FAILURE;
    }

    // Wait for the receiver thread to finish
    pthread_join(receiver_thread, NULL);

    // Clean up and close the buffer
    sbuffer_free(&buffer);

    return EXIT_SUCCESS;
}
