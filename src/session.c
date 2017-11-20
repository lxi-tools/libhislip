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
#include <pthread.h>
#include "session.h"
#include "error.h"

session_t session[MAX_SESSIONS] = {};
pthread_mutex_t session_mutex = PTHREAD_MUTEX_INITIALIZER;

int session_allocate(void)
{
    bool session_available = false;
    int i;

    pthread_mutex_lock(&session_mutex);

    // Find a free session entry (i)
    for (i=0; i<MAX_SESSIONS; i++)
    {
        if (session[i].allocated == false)
        {
            // Claim session
            session[i].allocated = true;
            session_available = true;
            break;
        }
    }

    pthread_mutex_unlock(&session_mutex);

    // Return error if no session can be allocated
    if (session_available == false)
    {
        error_printf("Too many active sessions!\n");
        return -1;
    }

    // Return session handle
    return i;
}

int session_free(int i)
{
    pthread_mutex_lock(&session_mutex);

    // Check session handle
    if ((i >= MAX_SESSIONS) || (i < 0))
    {
        error_printf("Invalid session handle");
        goto error;
    }

    // Check if already freed
    if (session[i].allocated == false)
    {
        error_printf("Error: Session already freed\n");
        goto error;
    }
   
    // Free session
    session[i].allocated = false;
    pthread_mutex_unlock(&session_mutex);
    return 0;

error:
    pthread_mutex_unlock(&session_mutex);
    return -1;
}
