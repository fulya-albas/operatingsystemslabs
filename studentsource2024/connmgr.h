#ifndef _CONNMGR_H_
#define _CONNMGR_H_

#include <stdio.h>
#include "lib/tcpsock.h"  // Assuming this provides TCP socket handling functions

/**
 * Initializes a TCP server that listens on a given port.
 *
 * \param server_ip: The IP address of the server to listen on.
 * \param server_port: The port number to listen on.
 * \param client: Pointer to store the client connection.
 * \return: TCP_NO_ERROR on success, an error code otherwise.
 */
int tcp_server_init(const char *server_ip, int server_port, tcpsock_t **client);

/**
 * Accepts a client connection on a TCP server.
 *
 * \param server_ip: The IP address to bind the server to.
 * \param server_port: The port to bind the server to.
 * \return: 0 if the server was successfully created, otherwise an error code.
 */
int tcp_active_listen(const char *server_ip, int server_port);

#endif /* _CONNMGR_H_ */
