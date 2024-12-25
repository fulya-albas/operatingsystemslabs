#ifndef CONNMGR_H
#define CONNMGR_H

#include <pthread.h>

// Define the shared buffer size and structure
#define BUFFER_SIZE 1024

// Shared buffer structure
typedef struct {
    char buffer[BUFFER_SIZE];
    int read_pos;
    int write_pos;
    pthread_mutex_t mutex;
} shared_buffer_t;

// Function prototypes
void init_shared_buffer(shared_buffer_t *buf);
void insert_into_buffer(shared_buffer_t *buf, const char *data, int len);
void *connmgr_socket_start(void *arg);

#endif // CONNMGR_H
