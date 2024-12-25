#include "sensor_db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>    // For pipe(), fork(), close(), read(), write()
#include <sys/wait.h>  // For waitpid()
#include <stdbool.h>   // For bool type
#include <stdint.h>    // For ssize_t

#define MAX_LOG_MSG 256



static int pipe_fd[2];
static pid_t logger_pid = 0;

FILE *open_db(char *filename, bool append) {
    if (create_log_process() == -1) {
        fprintf(stderr, "Logger creation failed.\n");
        return NULL;
    }

    FILE *f = fopen(filename, append ? "a" : "w");
    if (f) {
        write_to_log_process("Data file opened.\n");
    } else {
        write_to_log_process("Error opening data file.\n");
    }
    return f;
}

int insert_sensor(FILE *f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    if (!f) {
        write_to_log_process("Error: File pointer is NULL during data insertion.\n");
        return -1;
    }

    if (fprintf(f, "%u, %.6f, %ld\n", id, value, ts) < 0) {
        write_to_log_process("Error writing to data file.\n");
        return -1;
    }

    write_to_log_process("Data inserted.\n");
    fflush(f);
    return 0;
}

int close_db(FILE *f) {
    if (f) {
        fclose(f);
        write_to_log_process("Data file closed.\n");
        end_log_process();
        return 0;
    }
    return -1;
}

int write_to_log(int code, sensor_id_t id, sensor_value_t value) {
    log_payload payload = {code, id, value};
    if (write(pipe_fd[1], &payload, sizeof(log_payload)) == -1) {
        perror("write");
        return -1;
    }
    return 0;
}



int create_log_process() {
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        return -1;
    }

    logger_pid = fork();
    if (logger_pid == -1) {
        perror("fork");
        return -1;
    }

    if (logger_pid == 0) {
        // Child process: logger
        close(pipe_fd[1]); // Close write end in the child
        FILE *log_file = fopen("gateway.log", "a");
        if (!log_file) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        log_payload payload;
        int seq_num = 0;

        while (read(pipe_fd[0], &payload, sizeof(log_payload)) > 0) {
            time_t now = time(NULL);
            fprintf(log_file, "%d - %s - ", seq_num++, ctime(&now));

            switch (payload.code) {
                case LOG_NEW_CONNECTION:
                    fprintf(log_file, "Sensor node %u has established a connection.\n", payload.id);
                    break;
                case LOG_CLOSED_CONNECTION:
                    fprintf(log_file, "Sensor node %u disconnected.\n", payload.id);
                    break;
                case LOG_TIMEOUT:
                    fprintf(log_file, "Sensor node %u timed out.\n", payload.id);
                    break;
                case LOG_TOO_COLD:
                    fprintf(log_file, "Sensor node %u reports temperature too low (avg = %.2lf).\n", payload.id, payload.data);
                    break;
                case LOG_TOO_HOT:
                    fprintf(log_file, "Sensor node %u reports temperature too high (avg = %.2lf).\n", payload.id, payload.data);
                    break;
                case LOG_INVALID_ID:
                    fprintf(log_file, "Invalid ID received for sensor %u.\n", payload.id);
                    break;
                case LOG_NEW_DATA_FILE:
                    fprintf(log_file, "Created a new data file.\n");
                    break;
                case LOG_DATA_INSERT:
                    fprintf(log_file, "Data from sensor %u successfully inserted.\n", payload.id);
                    break;
                case LOG_DATA_FILE_CLOSED:
                    fprintf(log_file, "Data file has been closed.\n");
                    break;
                default:
                    fprintf(log_file, "Unknown log code: %d.\n", payload.code);
                    break;
            }

            fflush(log_file);
        }

        fclose(log_file);
        close(pipe_fd[0]);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        close(pipe_fd[0]); // Close read end in the parent
        return 0;
    }
}




int end_log_process() {
    if (write_to_log_process("END_LOG\n") == -1) {
        return -1;
    }
    close(pipe_fd[1]);
    waitpid(logger_pid, NULL, 0);
    return 0;
}
