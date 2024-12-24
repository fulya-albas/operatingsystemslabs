#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "sensor_db.h"
#include "sbuffer.h"
#include <inttypes.h>

#define LOG_FILE_NAME "gateway.log"
#define DB_FILE_NAME "data.csv"

#define READ_END 0
#define WRITE_END 1

FILE *log_file, *db_file; // File pointers for the log and DB.
int fd[2]; // Pipe file descriptors.

static int insert_sensor(sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    int res = fprintf(db_file, "%" PRIu16 ",%lf,%li\n", id, value, ts);
    fflush(db_file);
    return res;
}

int db_close() {
    return close(fd[WRITE_END]); // Important to let the child process die.
}

void *db_init() {
    db_file = fopen(DB_FILE_NAME, "w");
    if (db_file == NULL) {
        perror("File creation failed");
        return NULL;
    }
    log_pipe_write(LOG_NEW_DATA_FILE, 0, 0); // Log creation of new data file

    sbuffer_node_t *node = NULL;
    sensor_data_t *data;
    do {
        int res;
        do {
            res = sbuffer_read(&node, &data);
            usleep(10);
        } while (res == SBUFFER_NO_DATA);

        if (data->id == 0) break;

        if (insert_sensor(data->id, data->value, data->ts) < 0) {
            perror("Error writing to file");
            return NULL;
        }

        log_pipe_write(LOG_DATA_INSERT, data->id, 0); // Log data insertion
    } while (1);

    fclose(db_file);
    log_pipe_write(LOG_DATA_FILE_CLOSED, 0, 0); // Log closure of data file
    pthread_exit(NULL);
}

void log_pipe_write(log_codes code, sensor_id_t id, sensor_value_t data) {
    log_payload payload;
    memset(&payload, 0, sizeof(log_payload)); // Avoid unsafe behavior from padding.
    payload.id = id;
    payload.data = data;
    payload.code = code;
    write(fd[WRITE_END], &payload, sizeof(log_payload));
}

static void log_child_process() {
    close(fd[WRITE_END]);
    ssize_t n;
    log_payload payload;
    int last_log = 0;

    while ((n = read(fd[READ_END], &payload, sizeof(payload))) > 0) {
        last_log++;
        fprintf(log_file, "%i %lu ", last_log, (unsigned long)time(NULL));

        switch (payload.code) {
            case LOG_NEW_CONNECTION:
                fprintf(log_file, "Sensor node %i has opened a new connection.\n", payload.id);
                break;
            case LOG_CLOSED_CONNECTION:
                fprintf(log_file, "Sensor node %i has closed the connection.\n", payload.id);
                break;
            case LOG_TIMEOUT:
                fprintf(log_file, "Sensor node %i has timed-out.\n", payload.id);
                break;
            case LOG_TOO_COLD:
                fprintf(log_file, "Sensor node %i reports it’s too cold (avg temp = %lf).\n", payload.id, payload.data);
                break;
            case LOG_TOO_HOT:
                fprintf(log_file, "Sensor node %i reports it’s too hot (avg temp = %lf).\n", payload.id, payload.data);
                break;
            case LOG_INVALID_ID:
                fprintf(log_file, "Received sensor data with invalid sensor node ID %i.\n", payload.id);
                break;
            case LOG_NEW_DATA_FILE:
                fprintf(log_file, "A new data.csv file has been created.\n");
                break;
            case LOG_DATA_INSERT:
                fprintf(log_file, "Data insertion from sensor %i succeeded.\n", payload.id);
                break;
            case LOG_DATA_FILE_CLOSED:
                fprintf(log_file, "The data.csv file has been closed.\n");
                break;
        }
        fflush(log_file);
    }

    if (n == -1 || close(fd[READ_END]) == -1 || fclose(log_file) != 0) {
        perror("Error stopping logger");
    }

    _exit(EXIT_SUCCESS);
}

void log_init() {
    if (pipe(fd) == -1) {
        perror("Pipe creation failed");
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return;
    }

    if (pid != 0) {
        close(fd[READ_END]);
        return;
    }

    log_file = fopen(LOG_FILE_NAME, "w");
    if (log_file == NULL) {
        perror("Error creating log file");
        return;
    }

    log_child_process();
}
