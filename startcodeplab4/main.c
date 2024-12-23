#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include "sbuffer.h"
#include "config.h"

#define SENSOR_DATA_FILE "sensor_data"
#define OUTPUT_FILE "sensor_data_out.csv"
#define END_OF_STREAM_ID 2222
#define FILE_ERROR(fp, error_msg) do {               \
    if ((fp) == NULL) {                              \
        printf("%s\n", (error_msg));                 \
        exit(EXIT_FAILURE);                          \
    }                                                \
} while (0)

sbuffer_t *shared_buffer;

void write_to_csv(const sensor_data_t *data) {
    FILE *file = fopen("sensor_data.csv", "a");  // Open in append mode
    if (file == NULL) {
        perror("Failed to open CSV file");
        return;
    }

    // Write the sensor data to the file in CSV format
    fprintf(file, "%hu,%.4f,%ld\n", data->id, data->value, (long)data->ts);

    fclose(file);
}


void *writer_thread (void *arg ){
    printf("writer thread started\n");

    FILE *fp = fopen( SENSOR_DATA_FILE, "rb" );
    FILE_ERROR( fp , "failed 2 open sensor_data");
    sensor_data_t data ;
    while( fread ( &data.id, sizeof(sensor_id_t) , 1, fp)&& fread(&data.value, sizeof(sensor_value_t), 1, fp)&& fread( &data.ts, sizeof(sensor_ts_t), 1 , fp)){
        data.ts = time(NULL);
        printf("inserting data: id=%" PRIu16 ", value=%.2f, timestamp=%ld\n", data.id, data.value, (long)data.ts );

        sbuffer_insert( shared_buffer,&data);
        usleep(10000 );
    }
    data.id = END_OF_STREAM_ID;
    data.ts = time(NULL);
    sbuffer_insert( shared_buffer, &data);
    sbuffer_close(shared_buffer );
    fclose(fp );
    printf("writer thread is done \n");
    return NULL;

}

void *reader_thread( void *arg ){

    printf("reader started \n");

    FILE *fp = fopen( OUTPUT_FILE, "a");
    FILE_ERROR( fp, "failed 2 open sensor_data_out.csv" );
    sensor_data_t data;
    while (1){
        int res = sbuffer_remove( shared_buffer, &data);
        if( res == SBUFFER_NO_DATA && !buffer_is_open(shared_buffer)){
            break;
        }
        if( data.id != END_OF_STREAM_ID){
            fprintf(fp, "%" PRIu16 ",%.2f, %ld\n" , data.id, data.value, (long)data.ts );
            usleep(25000 );
        }

    }
    fclose( fp);


    printf("reader done \n");

    return NULL;
}

int main(){
    printf("main started \n");

    sbuffer_init(&shared_buffer);
    pthread_t writer, reader1, reader2;

    pthread_create( &writer, NULL, writer_thread, NULL);
    pthread_create( &reader1, NULL, reader_thread, NULL);
    pthread_create(&reader2, NULL, reader_thread, NULL);

    printf("threads made\n");

    pthread_join(writer, NULL);
    pthread_join(reader1, NULL);
    pthread_join( reader2, NULL);

    printf("threads joined \n");

    sbuffer_free(&shared_buffer);

    printf("main done\n");
    return 0;
}