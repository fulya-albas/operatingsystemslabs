#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LOG_MSG 256

static int pipe_fd[2];
static pid_t logger_pid = 0;

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
        // child process: logger
        close(pipe_fd[1]);
        FILE *log_file = fopen("gateway.log", "a");
        if (!log_file) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        char buffer[MAX_LOG_MSG];
        int seq_num = 0;

        while (1) {
            ssize_t bytes_read = read(pipe_fd[0], buffer, MAX_LOG_MSG - 1);
            if (bytes_read <= 0) {
                break;
            }

            buffer[bytes_read] = '\0';
            if (strcmp(buffer, "END_LOG\n") == 0) {
                break; // terminate logger
            }

            time_t now = time(NULL);
            fprintf(log_file, "%d - %s - %s", seq_num++, ctime(&now), buffer);
            fflush(log_file);
        }

        fclose(log_file);
        close(pipe_fd[0]);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        close(pipe_fd[0]);
        return 0;
    }
}

int write_to_log_process(char *msg) {
    if (logger_pid == 0 || write(pipe_fd[1], msg, strlen(msg)) == -1) {
        perror("write");
        return -1;
    }
    return 0;
}

int end_log_process() {
    if (write_to_log_process("END_LOG\n") == -1) {
        return -1;
    }
    close(pipe_fd[1]);
    waitpid(logger_pid, NULL, 0); //wait for logger to terminate
    return 0;
}
