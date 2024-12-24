#include "connmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lib/tcpsock.h"  // Assuming this provides the necessary TCP socket functions

/**
 * Initializes a TCP server to listen for incoming client connections.
 *
 * \param server_ip: The IP address of the server to listen on.
 * \param server_port: The port number to listen on.
 * \param client: Pointer to store the client connection.
 * \return: TCP_NO_ERROR on success, an error code otherwise.
 */
int tcp_server_init(const char *server_ip, int server_port, tcpsock_t **client) {
    if (tcp_active_open(client, server_port, server_ip) != TCP_NO_ERROR) {
        perror("Failed to open TCP connection");
        return -1;
    }
    return 0;
}

/**
 * Accepts an incoming client connection on the server.
 *
 * \param server_ip: The IP address to bind the server to.
 * \param server_port: The port to bind the server to.
 * \return: 0 if successful, or an error code.
 */
int tcp_active_listen(const char *server_ip, int server_port) {
    tcpsock_t *client;
    if (tcp_server_init(server_ip, server_port, &client) != 0) {
        return -1;
    }

    printf("Server listening on %s:%d...\n", server_ip, server_port);
    return 0;
}
