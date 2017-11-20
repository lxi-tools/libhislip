/*
 * Copyright (c) 2017  Martin Lund
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "error.h"

typedef struct
{
    int socket;
    void (*connection_callback)(int socket);

} connection_data_t;

static void print_data(void *data, int length)
{
    int i;
    char *bufferp;

    bufferp = data;
    (void)bufferp;

    for (i=0; i<length; i++)
    {
        if ((i%10 == 0) && (i !=0 ))
        {
            printf("\n");
            printf("%32s"," ");
        }
        printf("0x%02x ", (unsigned char) bufferp[i]);
    }
}

int tcp_connect(int *sd, char *address, int port, int timeout)
{
    struct sockaddr_in server_address;
    struct hostent *host;

    // Create a TCP/IP stream socket
    if ((*sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        error_printf("socket() call failed\n");
        return -1;
    }

    // Construct the server address structure
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family      = AF_INET;
    server_address.sin_port        = htons(port);
    server_address.sin_addr.s_addr = inet_addr(address);

    if (server_address.sin_addr.s_addr == (unsigned long) INADDR_NONE)
    {
        // Look up host address
        host = gethostbyname(address);

        if (host == (struct hostent *) NULL)
        {
            error_printf("Host not found\n");
            close(*sd);
            return -1;
        }

        memcpy(&server_address.sin_addr, host->h_addr, sizeof(server_address.sin_addr));
    }

    // Establish connection to server
    if (connect(*sd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    {
        error_printf("connect() call failed\n");
        close(*sd);
        return -1;
    }

    return 0;
}

int tcp_write(int sd, void *buffer, int length, int timeout)
{
    int status;
    struct timeval tv;
    fd_set wdfs;

    // Set timeout
    tv.tv_sec = 0;
    tv.tv_usec = timeout * 1000;

    FD_ZERO(&wdfs);
    FD_SET(sd, &wdfs);

    if (timeout)
        status = select(sd + 1, NULL, &wdfs, NULL, &tv);
    else
        status = select(sd + 1, NULL, &wdfs, NULL, NULL);
    if (status == -1)
        return -1;
    else if (status)
        return write(sd, buffer, length); // TODO: Write until all done
    else
        error_printf("Timeout\n");

    return -1;
}

int tcp_read(int sd, void *buffer, int length, int timeout)
{
    int status;
    struct timeval tv;
    fd_set rdfs;

    // Set timeout
    tv.tv_sec = 0;
    tv.tv_usec = timeout * 1000;

    FD_ZERO(&rdfs);
    FD_SET(sd, &rdfs);

    if (timeout)
        status = select(sd + 1, &rdfs, NULL, NULL, &tv);
    else
        status = select(sd + 1, &rdfs, NULL, NULL, NULL);
    if (status == -1)
        return -1;
    else if (status)
        return read(sd, buffer, length); // TODO: Read until all done
    else
        error_printf("Timeout\n");

    return -1;
}

int tcp_disconnect(int sd)
{
    return close(sd);
}

static void *connection_thread(void *arg)
{
    // Call connection callback
    connection_data_t *connection_data = arg;
    connection_data->connection_callback(connection_data->socket);
}

/*
 * tcp_server_start() - Start TCP server
 *
 * This will start a TCP server that listens for any incoming connections on
 * provided port. For each new incoming connection a callback is called in a
 * separate thread.
 *
 */

int tcp_server_start(int port, int n, void (*connection_callback)(int socket))
{
    int server_socket;
    int status, length = sizeof(int);
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    connection_data_t connection_data;

    // Create a reliable stream socket using TCP/IP
    if ((server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        error_printf("socket() call failed (%s)\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    // Assign server address to socket
    if ((status = bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address))) < 0)
    {
        error_printf("bind() call failed (%s)\n", strerror(errno));
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Allow up to N clients to be connected simultaneously
    if((status = listen(server_socket, n)) < 0)
    {
        error_printf("listen() call failed (%s)\n", strerror(errno));
        close(server_socket);
        exit (EXIT_FAILURE);
    }

    printf("Listening for incoming client connections on port %d\n", port);

    while (1)
    {
		pthread_t thread;
        int client_socket;

        // Wait for and accept incoming connection
        socklen_t sin_size = sizeof(struct sockaddr_in);
        if ((client_socket = accept(server_socket, (struct sockaddr *) &client_address, (socklen_t *) &sin_size)) < 0)
        {
            error_printf("accept() call failed (%s)\n", strerror(errno));
            close(server_socket);
            exit (EXIT_FAILURE);
        }

        printf("Incoming connection from client (%s)\n", inet_ntoa(client_address.sin_addr));

        // Prepare connection data
        connection_data.socket = client_socket;
        connection_data.connection_callback = connection_callback;    

        // Create server thread
	    pthread_create(&thread, NULL, connection_thread, &connection_data);

        // Make sure server thread does its own cleanup upon termination
    	pthread_detach(thread);
    }

    return 0;
}
