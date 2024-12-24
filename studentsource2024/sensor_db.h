#ifndef SENSOR_DB_H
#define SENSOR_DB_H

#include "config.h"  // Ensure sensor_data_t and related types are defined here
#include <stdint.h>

typedef enum {
    LOG_NEW_CONNECTION,
    LOG_CLOSED_CONNECTION,
    LOG_TOO_COLD,
    LOG_TOO_HOT,
    LOG_INVALID_ID,
    LOG_NEW_DATA_FILE,
    LOG_DATA_INSERT,
    LOG_DATA_FILE_CLOSED,
    LOG_TIMEOUT
} log_codes;

/**
 * Payload structure for logging information.
 */
typedef struct {
    log_codes code;
    sensor_id_t id;
    sensor_value_t data;
} log_payload;

/**
 * Initialize the Database.
 */
void *db_init();

/**
 * Close the Database and finish log processing.
 */
int db_close();

/**
 * Initialize the logging mechanism by forking the logger process.
 */
void log_init();

/**
 * Write a log entry to the pipe for logging.
 * @param code The log event type.
 * @param id The sensor ID.
 * @param data The data associated with the log event.
 */
void log_pipe_write(log_codes code, sensor_id_t id, sensor_value_t data);

#endif // SENSOR_DB_H
