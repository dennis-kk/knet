/*
 * Copyright (c) 2014-2015, dennis wang
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL dennis wang BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#ifdef TEST
#if TEST_ONE_LOOP

#include <stdio.h>
#include "knet.h"
#include "misc.h"

#define MAX_CLIENT 100
#define MAX_ECHO_COUNT MAX_CLIENT * 100

int recv_count = 0;
atomic_counter_t client_count = 0;

void client_cb(channel_ref_t* channel, channel_cb_event_e e) {
    char buffer[100] = {0};
    stream_t* stream = 0;
    int bytes = 0;
    if (e & channel_cb_event_recv) {
        stream = channel_ref_get_stream(channel);
        bytes = stream_pop(stream, buffer, sizeof(buffer));
        stream_push(stream, buffer, bytes);
        recv_count++;
        if (recv_count > MAX_ECHO_COUNT) {
            channel_ref_close(channel);
        }
    } else if (e & channel_cb_event_timeout) {
        printf("client close, timeout: %d\n", channel_ref_get_socket_fd(channel));
        channel_ref_close(channel);
    }
}

void connector_cb(channel_ref_t* channel, channel_cb_event_e e) {
    char buffer[100] = {0};
    stream_t* stream = 0;
    int bytes = 0;
    if (e & channel_cb_event_recv) {
        stream = channel_ref_get_stream(channel);
        bytes = stream_pop(stream, buffer, sizeof(buffer));
        stream_push(stream, buffer, bytes);
    } else if (e & channel_cb_event_close) {
        atomic_counter_dec(&client_count);
        if (client_count == 0) {
            printf("all client closed\n");
            loop_exit(channel_ref_get_loop(channel));
        }
    } else if (e & channel_cb_event_connect_timeout) {
        printf("connector close, timeout: %d\n", channel_ref_get_socket_fd(channel));
        channel_ref_close(channel);
    }
}

void acceptor_cb(channel_ref_t* channel, channel_cb_event_e e) {
    char buffer[100] = {0};
    stream_t* stream = 0;
    if (e & channel_cb_event_accept) {
        channel_ref_set_cb(channel, client_cb);
        stream = channel_ref_get_stream(channel);
        stream_push(stream, buffer, sizeof(buffer));
    }
}

#define TEST_TIMES 100

int main() {
    int i = 0;
    int error = 0;
    loop_t* loop = 0;
    channel_ref_t* acceptor = 0;
    channel_ref_t* connector = 0;
    int times = 0;

    loop = loop_create();
    acceptor = loop_create_channel(loop, 8, 1024 * 8);
    channel_ref_set_cb(acceptor, acceptor_cb);
    error = channel_ref_accept(acceptor, "127.0.0.1", 80, 5000);
    if (error_ok != error) {
        printf("channel_ref_accept failed: %d\n", error);
    }
    
    for (; times < TEST_TIMES; times++) {
        recv_count = 0;
        client_count = MAX_CLIENT;
        for (i = 0; i < MAX_CLIENT; i++) {
            connector = loop_create_channel(loop, 8, 8192);
            channel_ref_set_cb(connector, connector_cb);
            channel_ref_connect(connector, "127.0.0.1", 80, 2);
        }
        while (client_count > 0) {
            error = loop_run_once(loop);
        }
        if (error != error_ok) {
            printf("loop_run() failed: %d\n", error);
        }
    }
    loop_destroy(loop);
    return 0;
}

#endif /* TEST_ONE_LOOP */
#endif