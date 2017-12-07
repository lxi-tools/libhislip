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
#include <sys/queue.h>
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

#define SERVER_PROTOCOL_VERSION 0x100 // Major = 1, Minor = 0

typedef LIST_HEAD(subaddress_head_t, hs_subaddress_data_t) subaddress_head_t;
subaddress_head_t *subaddress_head;

static int server_subaddress_link(hs_server_t *server, char *subaddress, hs_subaddress_data_t *subaddress_data)
{
    bool match_found = false;
    hs_subaddress_data_t *sd;

    // Lookup subaddress in list of registered subaddresses
    LIST_FOREACH(sd, subaddress_head, entries)
    {
        if (strcmp(sd->subaddress, subaddress) == 0)
        {
            match_found = true;
            subaddress_data = sd;
            printf("found subaddress\n");
            break;
        }
    }

    // Link connection session to subaddress

    return match_found;
}

static void hs_process(int socket, hs_server_t *server)
{
    msg_header_t msg_header;
    int bytes_received, bytes_sent, i;
    char *subaddress;
    void *payload = NULL;

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
        if ((bytes_received = server->tcp_read(socket, &msg_header, MSG_HEADER_SIZE, 0)) == 0)
        {
            printf("Client closed connection\n");
            server->tcp_close(socket);
            return;
        }

        // Skip until we have enough bytes representing a message header
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
            // Check payload size
            if (msg_header.payload_length > server->config->payload_size_max)
            {
                error_printf("Maximum payload size exceeded\n");
                continue;
            }

            // Allocate payload receive buffer
            payload = malloc(msg_header.payload_length);
            if (payload == NULL)
            {
                error_printf("malloc() failed\n");
                continue;
            }

            // Read payload
            if ((bytes_received = server->tcp_read(socket, payload, msg_header.payload_length, 0)) == 0)
            {
                printf("Client closed connection\n");
                server->tcp_close(socket);
                return;
            }
        }

        // Perform action depending on message type
        switch (msg_header.type)
        {
            case Initialize:
                {
                    // Decode parameter field:
                    //  Client protocol version (upper)
                    //  Client vendor id (lower)
                    uint16_t *value = (uint16_t *) &msg_header.parameter;
                    uint16_t client_vendor_id = *value;
                    uint16_t client_protocol_version = *(value+1);

                    // Check if HiSlip protocol version is supported
                    if (client_protocol_version != SERVER_PROTOCOL_VERSION)
                    {
                        error_printf("Unsupported protocol version\n");
                        server->tcp_close(socket);
                        return;
                    }
                }

                // Create new connection session
                i = session_new();
                if (i < 0)
                {
                    error_printf("Could not allocate new session!\n");
                    server->tcp_close(socket);
                    return;
                }

                // Link connection session with registered subaddress callbacks
                subaddress = payload;
                if (server_subaddress_link(server, subaddress, session[i].subaddress_data) == -1)
                {
                    error_printf("Unable to link subaddress\n");
                    // TODO: Respond FatalError
                    continue;
                }

                // Send InitializeResponse message including
                //  SessionID
                //  Overlap-mode
                //  Server protocol version

                break;

            case InitializeResponse:
                break;
            case AsyncInitialize:
                break;
            case AsyncInitializeResponse:
                break;
            case Data:
                break;
            case DataEnd:
                break;
            case AsyncMaximumMessageSize:
                break;
            case AsyncMaximumMessageSizeResponse:
                break;
            case Error:
                break;
            case FatalError:
                break;
            default:
                break;
        }
    }
}

static void connection_callback(int socket, void *data)
{
    printf("client_socket = %d\n", socket);

    hs_process(socket, data);
}

int hs_server_run(hs_server_t *server)
{
    // Start server
    server->tcp_start(server->config->port, server->config->connections_max, connection_callback, server);

    return 0;
}

int hs_server_config_init(hs_server_config_t *config)
{
    // Initialize server configuration with default values
    config->port = HISLIP_PORT;
    config->connections_max = 1;
    config->worker_threads_max = 1;
    config->worker_queue_depth_max = 10;
    config->payload_size_max = 0x400000; // 4 MB
    config->message_timeout = 5000; // 5 seconds

    return 0;
}

int hs_server_init(hs_server_t *server, hs_server_config_t *config)
{
    // Intialize subaddress list
    subaddress_head = malloc(sizeof(subaddress_head_t));
    LIST_INIT(subaddress_head);

    // Set configuration
    server->config = config;

    // Configure TCP callbacks
    server->tcp_start = tcp_server_start;
    server->tcp_read = tcp_read;
    server->tcp_write = tcp_write;
    server->tcp_close = tcp_disconnect;

    return 0;
}

int hs_server_register_subaddress(hs_server_t *server, char *subaddress, hs_subaddress_callbacks_t *callbacks)
{
    // Add subaddres to list of registered subaddresses
    server->subaddress_data = malloc(sizeof(hs_subaddress_data_t));
    if (server->subaddress_data == NULL)
    {
        error_printf("Could not allocated space for new subaddress\n");
        return -1;
    }

    // Install subaddress data
    server->subaddress_data->callbacks = callbacks;
    server->subaddress_data->subaddress = subaddress;

    // Insert at list head
    LIST_INSERT_HEAD(subaddress_head, server->subaddress_data, entries);

    return 0;
}
