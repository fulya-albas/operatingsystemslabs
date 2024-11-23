#include "sensor_db.h"

FILE * open_db(char * filename, bool append) {
    const char *mode = append ? "a" : "w";
    FILE *file = fopen(filename, mode);
    return file;
}
int insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    if (f == NULL) {
        return -1; // File not open
    }
    fprintf(f, "%u, %.6f, %ld\n", id, value, ts);
    return 0;
}
int close_db(FILE * f) {
    if (f == NULL) {
        return -1; // File not open
    }
    return fclose(f) == 0 ? 0 : -1;
}
