#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>  // Add this header to fix the warning
#include "case_reversal.h"

#define BUFFER_SIZE 1024  // Maximum buffer size for the message

int main() {
    int pipefd[2];  // Array to hold pipe ends: pipefd[0] is read end, pipefd[1] is write end
    pid_t pid;
    char buffer[BUFFER_SIZE];

    // Create the pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork the process
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        close(pipefd[1]);  // Close unused write end

        // Read message from parent
        read(pipefd[0], buffer, BUFFER_SIZE);
        close(pipefd[0]);  // Close read end after reading

        // Reverse the case of the message
        reverse_case(buffer);

        // Print the result
        printf("Child Process Received: %s\n", buffer);
        exit(EXIT_SUCCESS);

    } else {
        // Parent process
        close(pipefd[0]);  // Close unused read end

        // Prepare the message
        const char *message = "Hi There";
        printf("Parent Process Sending: %s\n", message);

        // Write the message to the pipe
        write(pipefd[1], message, strlen(message) + 1);
        close(pipefd[1]);  // Close write end after writing

        // Wait for the child to complete
        wait(NULL);  // Wait for child process
    }

    return 0;
}
