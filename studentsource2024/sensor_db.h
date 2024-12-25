#ifndef SENSOR_DB_H
#define SENSOR_DB_H

#include "config.h"  // Ensure sensor_id_t, sensor_value_t, and sensor_ts_t are defined here
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

/**
 * Enumeration for log event codes.
 */
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
 * Structure for log payloads.
 */
typedef struct {
    log_codes code;      // Log event type
    sensor_id_t id;      // Sensor ID
    sensor_value_t data; // Associated data (e.g., temperature)
} log_payload;

/**
 * Opens the database file for logging sensor data.
 * @param filename The name of the file to open.
 * @param append Whether to append to the file (true) or overwrite (false).
 * @return Pointer to the opened file, or NULL on failure.
 */
FILE *open_db(char *filename, bool append);

/**
 * Inserts sensor data into the database file.
 * @param f Pointer to the database file.
 * @param id Sensor ID.
 * @param value Sensor value.
 * @param ts Timestamp for the data.
 * @return 0 on success, -1 on failure.
 */
int insert_sensor(FILE *f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts);

/**
 * Closes the database file and ends logging.
 * @param f Pointer to the database file.
 * @return 0 on success, -1 on failure.
 */
int close_db(FILE *f);

/**
 * Initializes the logging process by creating a logger subprocess.
 * @return 0 on success, -1 on failure.
 */
int create_log_process();

/**
 * Ends the logging process and waits for the logger subprocess to exit.
 * @return 0 on success, -1 on failure.
 */
int end_log_process();

/**
 * Writes a message to the logging process.
 * @param msg The message to log.
 * @return 0 on success, -1 on failure.
 */
int write_to_log_process(char *msg);

/**
 * Writes a log entry to the logger subprocess.
 * @param code The log event type.
 * @param id The sensor ID.
 * @param value The data associated with the log event.
 * @return 0 on success, -1 on failure.
 */
int write_to_log(int code, sensor_id_t id, sensor_value_t value);

#endif // SENSOR_DB_H
