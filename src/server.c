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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <hislip/server.h>
#include <hislip/common.h>
#include "tcp.h"
#include "message.h"
#include "error.h"
#include "session.h"

#define SERVER_PAYLOAD_MAX 0x500000 // 5 MB
#define SERVER_PROTOCOL_VERSION 0x100 // Major = 1, Minor = 0

static int server_subaddress_connect(hs_server_t *server, char *subaddress)
{
    // Connect session to subaddress
    return 0;
}

static void hs_process(int sd, hs_server_t *server)
{
    msg_header_t msg_header;
    int bytes_received, bytes_sent, i;
    void *payload;

    // Enter message processing loop
    while (1)
    {
        /* 1. Receive message (blocking, no timeout)
         * 1.1 Receive header
         * 1.2 Decode payload length
         * 1.3 Allocate payload length memory
         * 1.4 Receive payload length
         * 2. Decode message
         * 3. Execute request
         * 4. Send response (blocking, with timeout)
         */

        // Receive message header (blocking until data available)
        if ((bytes_received = server->tcp_read(sd, &msg_header, MSG_HEADER_SIZE, 0)) == 0)
        {
            printf("Client closed connection\n");
            server->tcp_close(sd);
            return;
        }

        // Skip until we have enough bytes representing a message header (non-progressive)
        if (bytes_received < MSG_HEADER_SIZE)
            continue;

        // Verify message header
        if (msg_header_verify(&msg_header))
        {
            // Invalid header
            error_printf("Invalid header\n");
            // Send FatalError message with error code 1 (Poorly formed message header)

            continue; // Skip until valid header received
        }

        // Receive any payload
        if (msg_header.payload_length > 0)
        {
            payload = malloc(msg_header.payload_length);
            if (payload == NULL)
            {
                error_printf("malloc() failed\n");
                continue;
            }

            if ((bytes_received = server->tcp_read(sd, payload, msg_header.payload_length, 0)) == 0)
            {
                printf("Client closed connection\n");
                server->tcp_close(sd);
                return;
            }
        }

        switch (msg_header.type)
        {
            char *subaddress;

            case Initialize:
                // Match subaddress to registered subaddress(es)
                //  If no match send FatalError
                subaddress = payload;
                if (server_subaddress_connect(server, subaddress) == -1) // Payload is subaddress
                {
                    error_printf("Unable to connect subaddress\n");
                    continue;
                }

                // Decode parameters:
                //  Client protocol version (upper)
                //  Client vendor id (lower)
                uint16_t *value = (uint16_t *) &msg_header.parameter;
                uint16_t client_vendor_id = *value;
                uint16_t client_protocol_version = *(value+1);

                // Check if HiSlip protocol version is supported
                if (client_protocol_version != SERVER_PROTOCOL_VERSION)
                {
                    error_printf("Unsupported protocol version\n");
                    return;
                }


                // Create new session
                i = session_new();
                if (i < 0)
                {
                    error_printf("Could not allocate new session!\n");
                    // Disconnect
                    return;
                }

                // Send InitializeResponse message including
                //  SessionID
                //  Overlap-mode
                //  Server protocol version

                break;
            case AsyncInitialize:
                break;
            default:
                break;
        }
    }
}

static void connection_callback(int sd, void *data)
{
    printf("client_socket = %d\n", sd);

    hs_process(sd, data);
}

int hs_server_run(hs_server_t *server)
{
    // Start server
    server->tcp_start(server->port, server->connections_max, connection_callback, server);

    return 0;
}

// hs_server_register_subaddress(hs_server_t *server, char *subaddress, callbacks);

int hs_server_init(hs_server_t *server, int port, int connections_max)
{
    // Configure options
    server->connections_max = connections_max;
    server->port = port;

    // Configure TCP callbacks
    server->tcp_start = tcp_server_start;
    server->tcp_read = tcp_read;
    server->tcp_write = tcp_write;
    server->tcp_close = tcp_disconnect;

    return 0;
}

int hs_server_register_subaddress(hs_server_t *server, char *subaddress, hs_subaddress_callbacks_t *callbacks)
{
    return 0;
}
