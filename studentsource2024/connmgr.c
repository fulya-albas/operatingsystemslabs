/**
* \author: Fulya Albas, student nr: r0913342, academic yr: 2024-2025
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "connmgr.h"
#include "config.h"
#include "lib/tcpsock.h"
#include "sbuffer.h"
#include <inttypes.h>
#include <pthread.h>
#include <errno.h>
#include "sensor_db.h"

/**
* The connection manager has the following tasks:
* - listens for data from the tcp socket
* - inserts incoming data into the shared buffer and logs the events
*/

static void *connmgr_start(void *client_socket){
    tcpsock_t *client = (tcpsock_t *)client_socket;
    int noofbytes;
    bool logged = false; //this will be used to ensure the connection will only be logged once
    sensor_id_t sensor_id=0;

    printf("Connection manager: STARTED CONNECTION-- %lu", pthread_self()); //for debugging

    do{
      int tcp_result; //result of tcp_recieve
      sensor_data_t *data = malloc(sizeof(sensor_data_t));
      memset(data, 0, sizeof(sensor_data_t));
      //start by reading the sensor id
      noofbytes = sizeof(sensor_id_t);
      tcp_result = tcp_receive(client, (void *) &data->id, &noofbytes, DTIMEOUT);
      ERROR_HANDLER(tcp_result == TCP_SOCKET_ERROR, "Error occured when reading data from tcp connection");
      //check if the sensor id was logged before
      if (!logged){
        write_to_log(LOG_NEW_CONNECTION, data->id, 0);
        sensor_id = data->id;
        logged = true;
      }
      //next read temperature
      noofbytes = sizeof(data->value);
      tcp_result = tcp_receive(client, (void *) &data->value, &noofbytes, DTIMEOUT);
      ERROR_HANDLER(tcp_result == TCP_SOCKET_ERROR, "Error occured when reading data from tcp connection");

      //next read sensor timestamp
      noofbytes = sizeof(data->ts);
      tcp_result = tcp_receive(client, (void *) &data->ts, &noofbytes, DTIMEOUT);
      ERROR_HANDLER(tcp_result == TCP_SOCKET_ERROR, "Error occured when reading data from tcp connection");

      //case if the client disconnects: will be logged with id
      if ((tcp_result = TCP_CONNECTION_CLOSED)){
        printf("client has disconnected");
        write_to_log(LOG_CLOSED_CONNECTION, sensor_id, 0);
        break;
      }

      //case if the client takes longer than DTIMEOUT as defined in makefile at runtime
      else if(errno == EAGAIN){
        printf("client took too long, time-out.");
        write_to_log(LOG_TIMEOUT, sensor_id, 0);
        break;
      }

      //if the number of bytes o fthe data receieved is not 0
      //and there is no timeout error
      //data gets inserted into the buffer
      else if((tcp_result == TCP_NO_ERROR) && noofbytes){
        printf("sensor id: = %" PRIu16 "  temperature = %g  timestamp = %ld", data->id, data->value, (long int) data->ts);
        sbuffer_insert(data);
      }
    } while (1);

    tcp_close(&client);
    return NULL;
}

void *connmgr_startup(void *port) {
  pthread_t tid[D_MAX_CONN];
  tcpsock_t *server, *client;
  int connections = 0;

  printf("Server is being started... %lu", pthread_self());
  ERROR_HANDLER((tcp_passive_open(&server, *((int *) port)) != TCP_NO_ERROR), "Error establishing connection between client and server.");
  do {

    ERROR_HANDLER(tcp_wait_for_connection(server, &client) != TCP_NO_ERROR, "Error establishing connection to client.");
    printf("incoming connection!");

    // Start a new thread whenever there is a new incoming connection. Stop when the connection counter == D_MAX_CONN
    pthread_create(&tid[connections], NULL, connmgr_socket_start, client);
    connections++;
  } while (connections < D_MAX_CONN);

  // Wait for all the clients to finish.
  for (int i = 0; i < D_MAX_CONN; ++i) {
    pthread_join(tid[i], NULL);
  }
  ERROR_HANDLER(tcp_close(&server) != TCP_NO_ERROR, "Error: couldn't close server.");

  printf("shutting server off!");

  // Insert an EOF marker to the buffer.
  sensor_data_t *data = malloc(sizeof(sensor_data_t));
  memset(data, 0, sizeof(sensor_data_t));
  sbuffer_insert(data);

  pthread_exit(NULL);
}

