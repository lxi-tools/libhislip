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

typedef struct
{
    int (*message_sync)(void *buffer, int length);
    int (*message_async)(void *buffer, int length);

} hs_subaddress_callbacks_t;

typedef struct
{
    char *subaddress;
    hs_subaddress_callbacks_t callbacks;

} hs_subaddress_data_t;

typedef struct
{
    int (*tcp_start)(int port, int n, void (*connection_callback)(int sd, void *data), void *data);
    int (*tcp_read)(int sd, void *buffer, int length, int timeout);
    int (*tcp_write)(int sd, void *buffer, int length, int timeout);
    int (*tcp_close)(int sd);
    
    int connections_max;
    int port;

    hs_subaddress_data_t subaddress_data[20];

} hs_server_t;

/* Server API */
int hs_server_init(hs_server_t *server, int port, int connections_max);
int hs_server_register_subaddress(hs_server_t *server, char *subaddress, hs_subaddress_callbacks_t *callbacks);
int hs_server_run(hs_server_t *server);

#endif
