#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include "config.h"
#include "lib/tcpsock.h"

#define LOG_FILE "server_log.txt"

/**
 * Logs sensor data to a file.
 */
void log_sensor_data(sensor_data_t data) {
    FILE *log_fp = fopen(LOG_FILE, "a");
    if (log_fp == NULL) {
        printf("Error: Unable to open log file.\n");
        return;
    }
    fprintf(log_fp, "Sensor ID: %" PRIu16 ", Temperature: %.2f, Timestamp: %ld\n",
            data.id, data.value, (long)data.ts);
    fclose(log_fp);
}

int main(int argc, char *argv[]) {
    tcpsock_t *server, *client;
    sensor_data_t data;
    int bytes, result;
    int conn_counter = 0;

    if (argc < 3) {
        printf("Usage: ./test_server <PORT> <MAX_CONNECTIONS>\n");
        return EXIT_FAILURE;
    }

    int PORT = atoi(argv[1]);
    int MAX_CONN = atoi(argv[2]);

    if (PORT <= 0 || PORT > 65535) {
        printf("Error: Port must be between 1 and 65535.\n");
        return EXIT_FAILURE;
    }

    printf("Test server starting on port %d, allowing up to %d connections.\n", PORT, MAX_CONN);

    // Initialize server socket
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) {
        printf("Error: Failed to open server socket.\n");
        return EXIT_FAILURE;
    }

    do {
        // Wait for an incoming client connection
        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) {
            printf("Error: Failed to accept client connection.\n");
            continue;
        }

        printf("Connection established with client %d.\n", conn_counter + 1);
        conn_counter++;

        // Read and process data from the client
        do {
            bytes = sizeof(data.id);
            result = tcp_receive(client, (void *)&data.id, &bytes);
            if (result != TCP_NO_ERROR || bytes == 0) break;

            bytes = sizeof(data.value);
            result = tcp_receive(client, (void *)&data.value, &bytes);
            if (result != TCP_NO_ERROR || bytes == 0) break;

            bytes = sizeof(data.ts);
            result = tcp_receive(client, (void *)&data.ts, &bytes);
            if (result != TCP_NO_ERROR || bytes == 0) break;

            printf("Sensor ID: %" PRIu16 ", Temperature: %.2f, Timestamp: %ld\n",
                   data.id, data.value, (long)data.ts);
            log_sensor_data(data);

        } while (result == TCP_NO_ERROR);

        if (result == TCP_CONNECTION_CLOSED) {
            printf("Client disconnected.\n");
        } else {
            printf("Error: Connection terminated unexpectedly.\n");
        }

        tcp_close(&client);

    } while (conn_counter < MAX_CONN);

    printf("Max connections reached. Shutting down server.\n");

    if (tcp_close(&server) != TCP_NO_ERROR) {
        printf("Error: Failed to close server socket.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
