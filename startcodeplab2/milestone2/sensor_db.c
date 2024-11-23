#include "sensor_db.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
