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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "message.h"
#include "error.h"

int msg_header_verify(msg_header_t *header)
{
    // Verify message prefix
    if (header->prologue != MSG_HEADER_PROLOGUE)
    {
        error_printf("Received invalid message header (invalid prologue)\n");
        return 1;
    }

    return 0;
}

int msg_create(
        void **message,
        msg_type_t type,
        uint8_t control_code,
        uint32_t parameter,
        uint64_t payload_length,
        void *payload)
{
    msg_header_t *header;
    char *payload_p;

    // Allocate memory for message buffer
    *message = malloc(sizeof(msg_header_t) + payload_length);
    if (*message == NULL)
    {
        error_printf("Failed to allocate memory for messaage\n");
        return -1;
    }

    // Create message header
    header = *message;
    header->prologue = MSG_HEADER_PROLOGUE;
    header->type = type;
    header->control_code = control_code;
    header->parameter = parameter;
    header->payload_length = payload_length;

    // Copy payload if any
    if (payload_length > 0)
    {
        payload_p = *message;
        memcpy(payload_p + sizeof(msg_header_t), payload, payload_length);
    }

    return 0;
}

void msg_destroy(void *message)
{
    free(message);
}

int msg_receive(void *message, int *length)
{
    // Receive header

    // Receive payload

    return 0;
}

int msg_send(void *message, int length)
{
    // Send message

    return 0;
}
