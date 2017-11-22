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
#include <hislip/client.h>
#include <hislip/common.h>
#include "tcp.h"
#include "session.h"
#include "error.h"

hs_client_t hs_connect(char *address, int port, char *subaddress, int timeout)
{
    int sd, i;

    // Create new session
    i = session_new();
    if (i < 0)
    {
        error_printf("Could not allocate new session!\n");
        goto error_session;
    }

    // Create TCP connection
    if (tcp_connect(&sd, address, port, timeout) != 0)
        goto error_connect;

    // Save sync channel socket
    session[i].socket_sync = sd;

    // Create Initialize message

    // send Initialize message

    // Wait for InitializeResponse message

    // session[i].SessionID = ...

    // Return client session handle
    return i;

error_connect:
    session_free(i);
error_session:
    return -1;
}

int hs_disconnect(hs_client_t client)
{
    tcp_disconnect(session[client].socket_sync);

    session_free(client);

    return 0;
}


int hs_send_receive_sync(hs_client_t client, void *message, int *length, int timeout)
{
    return 0;
}

int hs_send_receive_async(hs_client_t client, void *message, int length, int timeout,
        void (*receive_callback)(void *message, int length))
{
    return 0;
}
