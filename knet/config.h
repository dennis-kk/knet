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

#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#if defined(WIN32)
    #define _WIN32_WINNT 0X0500 /* TryEnterCriticalSection */
    #if defined(FD_SETSIZE)
        #undef FD_SETSIZE
        #define FD_SETSIZE 1024
    #else
        #define FD_SETSIZE 1024
    #endif /* defined(FD_SETSIZE) */
    #include <winsock2.h>
    #include <mswsock.h>
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <process.h>
    #if defined(_MSC_VER )
        #pragma comment(lib,"wsock32.lib")
    #endif /* defined(_MSC_VER) */
    #define socket_t SOCKET
    #define socket_len_t int
    #define thread_id_t uintptr_t
    #define atomic_counter_t volatile LONG
    #define uint16_t unsigned short
    #define uint32_t unsigned int
    #define uint64_t unsigned long long
#else
    #include <stdint.h>
    #include <errno.h>
    #include <netdb.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <pthread.h>
    #include <linux/tcp.h>
    #include <sys/epoll.h>
    #define socket_len_t socklen_t
    #define thread_id_t pthread_t
    #define socket_t int
    #define atomic_counter_t volatile int
#endif /* defined(WIN32) */

#ifndef INT_MAX
/* from stdint.h */
#define INT_MAX  2147483647 /* maximum (signed) int value */
#endif /* INT_MAX */

#define create(type)            (type*)malloc(sizeof(type))
#define create_raw(size)        (char*)malloc(size)
#define create_type(type, size) (type*)malloc(size)
#define destroy(ptr)            free(ptr)

#ifndef assert
    #if defined(DEBUG) || defined(_DEBUG)
        #define assert(expr) \
            do { \
                if (!expr) { \
                    fprintf(stderr, "assert failure @(%s:%d)\n", __FILE__, __LINE__); \
                    abort(); \
                } \
            } while (0);
    #else
        #define assert(expr)
    #endif /* defined(DEBUG) || defined(_DEBUG) */
#endif /* assert */

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

typedef struct _loop_t loop_t;
typedef struct _channel_t channel_t;
typedef struct _channel_ref_t channel_ref_t;
typedef struct _address_t address_t;
typedef struct _lock_t lock_t;
typedef struct _loop_balancer_t loop_balancer_t;
typedef struct _thread_runner_t thread_runner_t;
typedef struct _stream_t stream_t;
typedef struct _dlist_t dlist_t;
typedef struct _dlist_node_t dlist_node_t;
typedef struct _ringbuffer_t ringbuffer_t;
typedef struct _buffer_t buffer_t;
typedef struct _broadcast_t broadcast_t;

typedef enum _channel_event_e {
    channel_event_recv = 1,
    channel_event_send = 2,
} channel_event_e;

typedef enum _channel_state_e {
    channel_state_connect = 1, /* 主动发起连接，连接未完成 */
    channel_state_accept = 2,  /* 监听 */
    channel_state_close = 4,   /* 管道已关闭 */
    channel_state_active = 8,  /* 管道已激活，可以收发数据 */
} channel_state_e;

typedef enum _error_e {
    error_ok = 0,
    error_fail,
    error_recv_fail,
    error_send_fail,
    error_send_patial,
    error_recv_buffer_full,
    error_recv_nothing,
    error_connect_fail,
    error_bind_fail,
    error_listen_fail,
    error_ref_nonzero,
    error_loop_fail,
    error_loop_attached,
    error_loop_not_found,
    error_loop_impl_init_fail,
    error_thread_start_fail,
    error_already_close,
    error_impl_add_channel_ref_fail,
    error_getpeername,
    error_getsockname,
    error_not_correct_domain,
} error_e;

typedef enum _channel_cb_event_e {
    channel_cb_event_connect = 1,          /* 连接完成 */
    channel_cb_event_accept = 2,           /* 管道监听到了新连接请求 */ 
    channel_cb_event_recv = 4,             /* 管道有数据可以读 */
    channel_cb_event_send = 8,             /* 管道发送了字节，保留 */
    channel_cb_event_close = 16,           /* 管道关闭 */
    channel_cb_event_timeout = 32,         /* 管道读空闲 */
    channel_cb_event_connect_timeout = 64, /* 主动发起连接，但连接超时 */
} channel_cb_event_e;

typedef void (*thread_func_t)(thread_runner_t*);
typedef void (*channel_ref_cb_t)(channel_ref_t* channel, channel_cb_event_e e);

/* 根据需要， 开启不同选取器 */

#if defined(WIN32)
#define LOOP_IOCP 1    /* IOCP */
#define LOOP_SELECT 0  /* select */
#else
#define LOOP_EPOLL 1   /* epoll */
#define LOOP_SELECT 0  /* select */
#endif /* defined(WIN32) || defined(WIN64) */

#define TEST 1               /* 是否开启测试 */
#define TEST_ONE_LOOP 0      /* 单线程，单loop_t测试 */
#define TEST_MULTI_THREAD 1  /* 多线程，多loop_t测试 */

#endif /* CONFIG_H */
