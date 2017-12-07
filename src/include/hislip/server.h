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

#ifndef SERVER_H
#define SERVER_H

#include <sys/queue.h>

typedef struct
{
    int (*message_sync)(void *buffer, int length);
    int (*message_async)(void *buffer, int length);

} hs_subaddress_callbacks_t;

typedef struct hs_subaddress_data_t
{
    char *subaddress;
    hs_subaddress_callbacks_t *callbacks;
    LIST_ENTRY(hs_subaddress_data_t) entries;

} hs_subaddress_data_t;

typedef struct
{
    int port;
    int connections_max;
    int worker_threads_max;
    int worker_queue_depth_max;
    int payload_size_max;
    int message_timeout;

} hs_server_config_t;

typedef struct
{
    int (*tcp_start)(int port, int n, void (*connection_callback)(int socket, void *data), void *data);
    int (*tcp_read)(int socket, void *buffer, int length, int timeout);
    int (*tcp_write)(int socket, void *buffer, int length, int timeout);
    int (*tcp_close)(int socket);

    hs_server_config_t *config;
    hs_subaddress_data_t *subaddress_data;

} hs_server_t;

/* Server API */
int hs_server_init(hs_server_t *server, hs_server_config_t *config);
int hs_server_register_subaddress(hs_server_t *server, char *subaddress, hs_subaddress_callbacks_t *callbacks);
int hs_server_run(hs_server_t *server);

#endif
