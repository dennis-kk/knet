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

#ifdef LOOP_IOCP

#include "loop.h"
#include "list.h"
#include "channel_ref.h"
#include "misc.h"

#if defined(_MSC_VER )
    #pragma comment(lib,"Ws2_32.lib")
#endif /* defined(_MSC_VER) */

#define ACCEPTEX_ADDR_SIZE   sizeof(struct sockaddr_in) + 16
#define ACCEPTEX_BUFFER_SIZE 1024
#define ACCEPTEX             LPFN_ACCEPTEX

typedef enum _io_type_e {
    io_type_accept  = 1,
    io_type_connect = 2,
    io_type_recv    = 4,
    io_type_send    = 8,
} io_type_e;

typedef struct _per_io_t {  
    OVERLAPPED ov;  
    io_type_e  type;
} per_io_t;

typedef struct _AcceptEx_t {
    ACCEPTEX       fn_AcceptEx;                  /* AcceptEx函数指针 */
    socket_t       socket_fd;                    /* 当前未决的客户端套接字 - AcceptEx */
    char           buffer[ACCEPTEX_BUFFER_SIZE]; /* 参数 - AcceptEx */
} AcceptEx_t;

typedef struct _per_sock_t {  
    channel_ref_t* channel_ref;                  /* 当前管道 */
    AcceptEx_t*    AcceptEx_info;                /* AcceptEx_t指针 */
    per_io_t       io_recv;                      /* 当前实现只支持同一个时刻只投递一个recv请求 */
    per_io_t       io_send;                      /* 当前实现只支持同一个时刻只投递一个send请求 */
} per_sock_t;

typedef struct _loop_iocp_t {
    HANDLE iocp;
} loop_iocp_t;

per_sock_t* socket_data_create();
void socket_data_destroy(per_sock_t* data);
ACCEPTEX get_fn_AcceptEx(socket_t fd);
AcceptEx_t* socket_data_prepare_accept(per_sock_t* data);
loop_iocp_t* get_impl(loop_t* loop);
per_sock_t* get_data(channel_ref_t* channel_ref);
void on_iocp_recv(channel_ref_t* channel_ref);
void on_iocp_send(channel_ref_t* channel_ref);

per_sock_t* socket_data_create() {
    per_sock_t* data = create(per_sock_t);
    assert(data);
    memset(data, 0, sizeof(per_sock_t));
    return data;
}

void socket_data_destroy(per_sock_t* data) {
    assert(data);
    if (data->AcceptEx_info) {
        /* 关闭套接字 */
        if (data->AcceptEx_info->socket_fd) {
            socket_close(data->AcceptEx_info->socket_fd);
        }
        destroy(data->AcceptEx_info);
    }
    destroy(data);
}

AcceptEx_t* socket_data_prepare_accept(per_sock_t* data) {
    socket_t fd = 0;
    assert(data);
    fd = channel_ref_get_socket_fd(data->channel_ref);
    if (!data->AcceptEx_info) {
        data->AcceptEx_info = create(AcceptEx_t);
        assert(data->AcceptEx_info);
        memset(data->AcceptEx_info, 0, sizeof(AcceptEx_t));
    }
    if (!data->AcceptEx_info->fn_AcceptEx) {
        data->AcceptEx_info->fn_AcceptEx = get_fn_AcceptEx(fd);
    }
    /* 建立一个支持重叠I/O的套接字 */
    data->AcceptEx_info->socket_fd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
    if (data->AcceptEx_info->socket_fd == INVALID_SOCKET) {
        assert(0);
        return 0;
    }
    return data->AcceptEx_info;
}

loop_iocp_t* get_impl(loop_t* loop) {
    return (loop_iocp_t*)loop_get_impl(loop);
}

per_sock_t* get_data(channel_ref_t* channel_ref) {
    return (per_sock_t*)channel_ref_get_data(channel_ref);
}

int impl_create(loop_t* loop) {
    WSADATA wsa;
    loop_iocp_t* impl = create(loop_iocp_t);
    assert(impl);
    memset(impl, 0, sizeof(loop_iocp_t));
    loop_set_impl(loop, impl);
    impl->iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (!impl->iocp) {
        destroy(impl);
        return error_loop_impl_init_fail;
    }
    WSAStartup(MAKEWORD(2, 2), &wsa);
    return error_ok;
}

void impl_destroy(loop_t* loop) {
    loop_iocp_t* impl = get_impl(loop);
    assert(impl);
    CloseHandle(impl->iocp);
    destroy(impl);
    WSACleanup();
}

int _select(loop_t* loop, time_t ts) {
    BOOL           error       = FALSE;
    DWORD          bytes       = 0;
    DWORD          last_error  = 0;
    per_io_t*      per_io      = 0;
    per_sock_t*    per_sock    = 0;
    channel_ref_t* channel_ref = 0;
    loop_iocp_t*   impl        = get_impl(loop);
    error = GetQueuedCompletionStatus(impl->iocp, &bytes, (PULONG_PTR)&per_sock, (LPOVERLAPPED*)&per_io, 1);
    if (error == FALSE) {
        last_error = GetLastError();
        if ((last_error == WAIT_TIMEOUT) || (last_error == ERROR_NETNAME_DELETED) || (last_error == ERROR_OPERATION_ABORTED)) {
            return error_ok;
        } else {
            assert(0);
        }
        return error_loop_fail;
    }
    channel_ref = per_sock->channel_ref;    
    assert(channel_ref);
    if ((per_io->type & io_type_recv) || (per_io->type & io_type_accept)) {
        channel_ref_update(channel_ref, channel_event_recv, ts);
    } else if ((per_io->type & io_type_send) || (per_io->type & io_type_connect)) {
        channel_ref_update(channel_ref, channel_event_send, ts);
    }
    return error_ok;
}

int impl_run_once(loop_t* loop) {
    int    error = 0;
    time_t ts    = time(0);
    assert(loop);
    error = _select(loop, ts);
    if (error != error_ok) {
        return error;
    }
    loop_check_timeout(loop, ts);
    loop_check_close(loop);
    return error_ok;
}

socket_t impl_channel_accept(channel_ref_t* channel_ref) {
    socket_t    acceptor = 0;
    per_sock_t* per_sock = 0;
    socket_t    client   = 0;
    assert(channel_ref);
    acceptor = channel_ref_get_socket_fd(channel_ref);
    assert(acceptor);
    per_sock = (per_sock_t*)channel_ref_get_data(channel_ref);
    assert(per_sock);
    assert(per_sock->AcceptEx_info);
    client = per_sock->AcceptEx_info->socket_fd;
    /* MSDN: On Windows XP and later, once the AcceptEx function completes and the SO_UPDATE_ACCEPT_CONTEXT
       option is set on the accepted socket, the local address associated with the accepted socket can also
       be retrieved using the getsockname function. Likewise, the remote address associated with the accepted
       socket can be retrieved using the getpeername function.
     */
    setsockopt(client, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&acceptor, sizeof(socket_t));
    return client;
}

ACCEPTEX get_fn_AcceptEx(socket_t fd) {
    int      error         = 0;
    DWORD    bytes         = 0;
    ACCEPTEX fn_AcceptEx   = 0;
    GUID     guid_AcceptEx = WSAID_ACCEPTEX;
    /* 获取AcceptEx指针 */
    error = WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_AcceptEx, sizeof(guid_AcceptEx),
                &fn_AcceptEx, sizeof(ACCEPTEX), &bytes, NULL, NULL);
    if (error) {
        assert(0);
        return 0;
    }
    return fn_AcceptEx;
}

void on_iocp_recv(channel_ref_t* channel_ref) {
    DWORD          bytes           = 0;
    DWORD          flags           = 0;
    DWORD          error           = 0;
    WSABUF         sbuf            = {0, 0};
    BOOL           result          = FALSE;
    socket_t       fd              = channel_ref_get_socket_fd(channel_ref);
    int            flag            = channel_ref_get_flag(channel_ref);
    per_sock_t*    per_sock        = (per_sock_t*)channel_ref_get_data(channel_ref);
    AcceptEx_t*    AcceptEx_ptr    = 0;
    per_io_t*      per_io          = &per_sock->io_recv;
    if (channel_ref_check_state(channel_ref, channel_state_accept)) {
        per_io->type = io_type_accept;
        AcceptEx_ptr = socket_data_prepare_accept(per_sock);
        /* 投递一个accept请求 */
        result = AcceptEx_ptr->fn_AcceptEx(fd, AcceptEx_ptr->socket_fd, AcceptEx_ptr->buffer,
                    0, ACCEPTEX_ADDR_SIZE, ACCEPTEX_ADDR_SIZE, 0, &per_io->ov);
        if (result == FALSE) {
            error = GetLastError();
            if (error != ERROR_IO_PENDING) {
                assert(0);
            }
            return;
        }
    } else {
        per_io->type = io_type_recv;
        /* 投递一个0长度recv请求 */
        result = WSARecv(fd, &sbuf, 1, &bytes, &flags, &per_io->ov, 0);
        if (result != 0) {
            error = GetLastError();
            if ((error != ERROR_IO_PENDING) && (error != WSAENOTCONN)) {
                assert(0);
            }
            return;
        }
    }
    flag |= io_type_recv;
    channel_ref_set_flag(channel_ref, flag);
}

void on_iocp_send(channel_ref_t* channel_ref) {
    DWORD          bytes    = 0;
    DWORD          flags    = 0;
    DWORD          error    = 0;
    WSABUF         sbuf     = {0, 0};
    int            result   = FALSE;
    socket_t       fd       = channel_ref_get_socket_fd(channel_ref);
    int            flag     = channel_ref_get_flag(channel_ref);
    per_sock_t*    per_sock = (per_sock_t*)channel_ref_get_data(channel_ref);
    per_io_t*      per_io   = &per_sock->io_send;
    /* 设置投递事件类型 */
    if (channel_ref_check_state(channel_ref, channel_state_connect)) {
        per_io->type = io_type_connect;
    } else {
        per_io->type = io_type_send;
    }
    /* 投递一个0长度send请求 */
    result = WSASend(fd, &sbuf, 1, &bytes, flags, &per_io->ov, 0);
    if (result != 0) {
        error = GetLastError();
        if ((error != ERROR_IO_PENDING) && (error != WSAENOTCONN)) {
            assert(0);
        }
        return;
    }
    /* 设置投递标志 */
    flag |= io_type_send;
    channel_ref_set_flag(channel_ref, flag);
}

int impl_event_add(channel_ref_t* channel_ref, channel_event_e e) {
    assert(channel_ref);
    if (channel_ref_check_state(channel_ref, channel_state_close)) {
        return error_already_close;
    }
    /* 投递事件 */
    if (channel_event_recv & e) {
        on_iocp_recv(channel_ref);
    } else if (e & channel_event_send) {
        on_iocp_send(channel_ref);
    }
    return error_ok;
}

int impl_event_remove(channel_ref_t* channel_ref, channel_event_e e) {
    int flag = 0;
    e;
    assert(channel_ref);
    flag = channel_ref_get_flag(channel_ref);
    if (flag & io_type_recv) {
        flag &= ~io_type_recv;
    } else if (flag & io_type_send) {
        flag &= ~io_type_send;
    }
    channel_ref_set_flag(channel_ref, flag);
    return error_ok;
}

int impl_add_channel_ref(loop_t* loop, channel_ref_t* channel_ref) {
    loop_iocp_t* impl      = 0;
    socket_t     socket_fd = 0;
    HANDLE       iocp      = 0;
    per_sock_t*  per_sock  = 0;
    assert(loop);
    assert(channel_ref);
    impl      = get_impl(loop);
    socket_fd = channel_ref_get_socket_fd(channel_ref);
    iocp      = 0;
    per_sock  = socket_data_create();
    assert(per_sock);
    per_sock->channel_ref  = channel_ref;
    /* 与IOCP关联 */
    iocp = CreateIoCompletionPort((HANDLE)socket_fd, impl->iocp, (ULONG_PTR)per_sock, 0);
    if (!iocp) {
        socket_data_destroy(per_sock);
        return error_impl_add_channel_ref_fail;
    }
    channel_ref_set_data(channel_ref, per_sock);
    return error_ok;
}

int impl_remove_channel_ref(loop_t* loop, channel_ref_t* channel_ref) {
    /* 外部关闭套接字后，IOCP解除关联，channel_ret_t销毁和loop_t退出的时候调用此函数 */
    per_sock_t* per_sock = 0;
    loop;
    assert(channel_ref);
    assert(loop);
    per_sock = get_data(channel_ref);
    assert(per_sock);
    socket_data_destroy(per_sock);
    channel_ref_set_data(channel_ref, 0);
    return error_ok;
}

#endif
