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
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
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
#include "address.h"
#include "misc.h"
#include "logger.h"

#if defined(_MSC_VER )
    #pragma comment(lib,"Ws2_32.lib")
#endif /* defined(_MSC_VER) */

#define ACCEPTEX_ADDR_SIZE   sizeof(struct sockaddr_in6) + 16 /* AcceptEx函数参数，详见MSDN */
#define ACCEPTEX_BUFFER_SIZE 1024                            /* AcceptEx函数参数，详见MSDN */
#define ACCEPTEX             LPFN_ACCEPTEX                   /* AcceptEx函数指针，详见MSDN */

/**
 * 事件类型
 */
typedef enum _io_type_e {
    io_type_accept  = 1,  /* 新连接到来 */
    io_type_connect = 2,  /* 连接建立 */
    io_type_recv    = 4,  /* 接收 */
    io_type_send    = 8,  /* 发送 */
    io_type_close   = 16, /* 管道关闭 */
} io_type_e;

/**
 * per-I/O数据
 */
typedef struct _per_io_t {  
    OVERLAPPED ov;   /* windows重叠I/O所需数据结构 */
    io_type_e  type; /* 当前I/O类型 */
} per_io_t;

/**
 * AcceptEx函数相关
 */
typedef struct _AcceptEx_t {
    ACCEPTEX       fn_AcceptEx;                  /* AcceptEx函数指针 */
    socket_t       socket_fd;                    /* 当前未决的客户端套接字 - AcceptEx */
    char           buffer[ACCEPTEX_BUFFER_SIZE]; /* 参数 - AcceptEx */
} AcceptEx_t;

/**
 * per-socket数据
 */
typedef struct _per_sock_t {  
    kchannel_ref_t* channel_ref;  /* 当前管道 */
    AcceptEx_t*    AcceptEx_info; /* AcceptEx_t指针 */
    per_io_t       io_recv;       /* 当前实现只支持同一个时刻只投递一个recv请求 */
    per_io_t       io_send;       /* 当前实现只支持同一个时刻只投递一个send请求 */
} per_sock_t;

/**
 * IOCP实现
 */
typedef struct _loop_iocp_t {
    HANDLE iocp; /* IOCP句柄 */
} loop_iocp_t;

/**
 * 建立per-socket数据
 * @return per_sock_t实例
 */
per_sock_t* socket_data_create();

/**
 * 销毁per-socket数据
 * @param data per_sock_t实例
 */
void socket_data_destroy(per_sock_t* data);

/**
 * 取得管道的per-socket数据
 * @param channel_ref 管道引用
 * @return per_sock_t实例
 */
per_sock_t* get_data(kchannel_ref_t* channel_ref);

/**
 * 获取监听的描述符的AcceptEx函数指针
 * @return AcceptEx函数指针
 */
ACCEPTEX get_fn_AcceptEx(socket_t fd);

/**
 * 准备接受新的连接
 * @return AcceptEx函数指针
 */
AcceptEx_t* socket_data_prepare_accept(per_sock_t* data);

/**
 * 取得网络循环的IOCP实现
 * @return loop_iocp_t实例
 */
loop_iocp_t* get_impl(kloop_t* loop);

/**
 * 注册RECV事件
 * @param channel_ref kchannel_ref_t实例
 */
void on_iocp_recv(kchannel_ref_t* channel_ref);

/**
 * 注册SEND事件
 * @param channel_ref kchannel_ref_t实例
 */
void on_iocp_send(kchannel_ref_t* channel_ref);

per_sock_t* socket_data_create() {
    per_sock_t* data = create(per_sock_t);
    verify(data);
    memset(data, 0, sizeof(per_sock_t));
    return data;
}

void socket_data_destroy(per_sock_t* data) {
    verify(data);
    if (data->AcceptEx_info) {
        /* 关闭套接字 */
        if (data->AcceptEx_info->socket_fd) {
            socket_close(data->AcceptEx_info->socket_fd);
        }
        knet_free(data->AcceptEx_info);
    }
    knet_free(data);
}

AcceptEx_t* socket_data_prepare_accept(per_sock_t* data) {
    socket_t fd = 0;
    verify(data);
    fd = knet_channel_ref_get_socket_fd(data->channel_ref);
    if (!data->AcceptEx_info) {
        data->AcceptEx_info = create(AcceptEx_t);
        verify(data->AcceptEx_info);
        memset(data->AcceptEx_info, 0, sizeof(AcceptEx_t));
    }
    if (!data->AcceptEx_info->fn_AcceptEx) {
        data->AcceptEx_info->fn_AcceptEx = get_fn_AcceptEx(fd);
    }
    /* 建立一个支持重叠I/O的套接字 */
    data->AcceptEx_info->socket_fd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
    if (data->AcceptEx_info->socket_fd == INVALID_SOCKET) {
        verify(0);
        return 0;
    }
    return data->AcceptEx_info;
}

loop_iocp_t* get_impl(kloop_t* loop) {
    return (loop_iocp_t*)knet_loop_get_impl(loop);
}

per_sock_t* get_data(kchannel_ref_t* channel_ref) {
    return (per_sock_t*)knet_channel_ref_get_data(channel_ref);
}

int knet_impl_create(kloop_t* loop) {
    WSADATA wsa;
    loop_iocp_t* impl = create(loop_iocp_t);
    verify(impl);
    memset(impl, 0, sizeof(loop_iocp_t));
    knet_loop_set_impl(loop, impl);
    impl->iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (!impl->iocp) {
        knet_free(impl);
        return error_loop_impl_init_fail;
    }
    WSAStartup(MAKEWORD(2, 2), &wsa);
    return error_ok;
}

void knet_impl_destroy(kloop_t* loop) {
    loop_iocp_t* impl = get_impl(loop);
    verify(impl);
    CloseHandle(impl->iocp);
    knet_free(impl);
    WSACleanup();
}

int _select(kloop_t* loop, time_t ts) {
    BOOL            error       = FALSE;
    DWORD           bytes       = 0;
    DWORD           last_error  = 0;
    per_io_t*       per_io      = 0;
    per_sock_t*     per_sock    = 0;
    kchannel_ref_t* channel_ref = 0;
    loop_iocp_t*    impl        = get_impl(loop);
    error = GetQueuedCompletionStatus(impl->iocp, &bytes, (PULONG_PTR)&per_sock, (LPOVERLAPPED*)&per_io, 1);
    last_error = GetLastError();
    if (FALSE == error) {
        if (last_error == WAIT_TIMEOUT) {
            return error_ok;
        }
        if (last_error == ERROR_OPERATION_ABORTED) {
            /* 取消的操作 */
            verify(per_sock);
            knet_channel_ref_decref(per_sock->channel_ref);
            return error_ok;
        }
    }
    verify(per_sock);
    verify(per_io);
    channel_ref = per_sock->channel_ref;  
    verify(channel_ref);
    if (!knet_channel_ref_check_state(channel_ref, channel_state_close)) {
        if ((per_io->type & io_type_recv) || (per_io->type & io_type_accept)) {
            knet_channel_ref_update(channel_ref, channel_event_recv, ts);
        } else if ((per_io->type & io_type_send) || (per_io->type & io_type_connect)) {
            knet_channel_ref_update(channel_ref, channel_event_send, ts);
        }
    }
    /* 管道事件处理完毕，减少引用计数 */
    knet_channel_ref_decref(channel_ref);
    return error_ok;
}

int knet_impl_run_once(kloop_t* loop) {
    int    error = 0;
    time_t ts    = time(0);
    verify(loop);
    error = _select(loop, ts);
    if (error != error_ok) {
        return error;
    }
    knet_loop_check_timeout(loop, ts);
    knet_loop_check_close(loop);
    return error_ok;
}

socket_t knet_impl_channel_accept(kchannel_ref_t* channel_ref) {
    socket_t    acceptor = 0;
    per_sock_t* per_sock = 0;
    socket_t    client   = 0;
    verify(channel_ref);
    acceptor = knet_channel_ref_get_socket_fd(channel_ref);
    verify(acceptor);
    per_sock = (per_sock_t*)knet_channel_ref_get_data(channel_ref);
    verify(per_sock);
    verify(per_sock->AcceptEx_info);
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
        verify(0);
        return 0;
    }
    return fn_AcceptEx;
}

void on_iocp_recv(kchannel_ref_t* channel_ref) {
    DWORD          bytes           = 0;
    DWORD          flags           = 0;
    DWORD          error           = 0;
    WSABUF         sbuf            = {0, 0};
    BOOL           result          = FALSE;
    socket_t       fd              = knet_channel_ref_get_socket_fd(channel_ref);
    int            flag            = knet_channel_ref_get_flag(channel_ref);
    per_sock_t*    per_sock        = (per_sock_t*)knet_channel_ref_get_data(channel_ref);
    AcceptEx_t*    AcceptEx_ptr    = 0;
    per_io_t*      per_io          = &per_sock->io_recv;
    if (knet_channel_ref_check_state(channel_ref, channel_state_close)) {
        return;
    }
    if (knet_channel_ref_check_state(channel_ref, channel_state_accept)) {
        per_io->type = io_type_accept;
        AcceptEx_ptr = socket_data_prepare_accept(per_sock);
        /* 投递一个accept请求 */
        result = AcceptEx_ptr->fn_AcceptEx(fd, AcceptEx_ptr->socket_fd, AcceptEx_ptr->buffer,
                    0, ACCEPTEX_ADDR_SIZE, ACCEPTEX_ADDR_SIZE, 0, &per_io->ov);
        if (result == FALSE) {
            error = GetLastError();
            if (error != ERROR_IO_PENDING) {
                knet_channel_ref_close(channel_ref);
            }
            /* 增加引用计数 */
            knet_channel_ref_incref(channel_ref);
            return;
        }
    } else {
        per_io->type = io_type_recv;
        /* 投递一个0长度recv请求 */
        result = WSARecv(fd, &sbuf, 1, &bytes, &flags, &per_io->ov, 0);
        if (result != 0) {
            error = GetLastError();
            if ((error != ERROR_IO_PENDING) && (error != WSAENOTCONN)) {
                knet_channel_ref_close(channel_ref);
            }
            /* 增加引用计数 */
            knet_channel_ref_incref(channel_ref);
            return;
        }
    }
    /* 增加引用计数 */
    knet_channel_ref_incref(channel_ref);
    flag |= io_type_recv;
    knet_channel_ref_set_flag(channel_ref, flag);
}

typedef BOOL(WINAPI *CONNECTEX)(SOCKET, const struct sockaddr *, int, PVOID, DWORD, LPDWORD, LPOVERLAPPED);

void on_iocp_send(kchannel_ref_t* channel_ref) {
    DWORD          bytes    = 0;
    DWORD          flags    = 0;
    DWORD          error    = 0;
    WSABUF         sbuf     = {0, 0};
    int            result   = FALSE;
    socket_t       fd       = knet_channel_ref_get_socket_fd(channel_ref);
    int            flag     = knet_channel_ref_get_flag(channel_ref);
    per_sock_t*    per_sock = (per_sock_t*)knet_channel_ref_get_data(channel_ref);
    per_io_t*      per_io   = &per_sock->io_send;
    CONNECTEX      fn_ConnectEx = 0;
    GUID           guid_ConnectEx = WSAID_CONNECTEX;
    struct         sockaddr_in6 in6;
    struct         sockaddr_in  in;
    struct         sockaddr* sockaddr_ptr = 0;
    struct         sockaddr_in6 bind_in6;
    struct         sockaddr_in bind_in;
    struct         sockaddr* bind_sockaddr_ptr = 0;
    int            addr_len = 0;
    BOOL           connect_ret = FALSE;
    kaddress_t*    peer_address = 0;
    int            bind_ret = 0;
    if (knet_channel_ref_check_state(channel_ref, channel_state_close)) {
        return;
    }
    /* 设置投递事件类型 */
    if (knet_channel_ref_check_state(channel_ref, channel_state_connect)) {
        per_io->type = io_type_connect;
    } else {
        per_io->type = io_type_send;
    }
    /* 连接状态的管道，检查是否已经完成 */
    if (knet_channel_ref_check_state(channel_ref, channel_state_connect)) {
        error = WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_ConnectEx, sizeof(guid_ConnectEx),
            &fn_ConnectEx, sizeof(CONNECTEX), &bytes, NULL, NULL);
        if (error) {
            knet_channel_ref_close(channel_ref);
            return;
        }
        peer_address = knet_channel_ref_get_peer_address(channel_ref);
        if (!peer_address) {
            knet_channel_ref_close(channel_ref);
            return;
        }
        if (knet_channel_ref_is_ipv6(channel_ref)) {
            memset(&in6, 0, sizeof(in6));
            in6.sin6_family = AF_INET6;
            in6.sin6_port = htons((unsigned short)address_get_port(peer_address));
            in6.sin6_addr = in6addr_any;
            if (address_get_ip(peer_address)) {
                inet_pton(AF_INET6, address_get_ip(peer_address), &in6.sin6_addr);
            }
            sockaddr_ptr = (struct sockaddr*)&in6;
            addr_len = sizeof(in6);
            memset(&bind_in6, 0, sizeof(bind_in6));
            bind_in6.sin6_family = AF_INET6;
            bind_in6.sin6_port = 0;
            bind_in6.sin6_addr = in6addr_any;
            bind_sockaddr_ptr = (struct sockaddr*)&bind_in6;
        } else {
            memset(&in, 0, sizeof(in));
            in.sin_family = AF_INET;
            in.sin_port = htons((unsigned short)address_get_port(peer_address));
            in.sin_addr.S_un.S_addr = INADDR_ANY;
            if (address_get_ip(peer_address)) {
                in.sin_addr.S_un.S_addr = inet_addr(address_get_ip(peer_address));
            }
            sockaddr_ptr = (struct sockaddr*)&in;
            addr_len = sizeof(in);
            memset(&bind_in, 0, sizeof(bind_in));
            bind_in.sin_family = AF_INET;
            bind_in.sin_port = 0;
            bind_in.sin_addr.S_un.S_addr = INADDR_ANY;
            bind_sockaddr_ptr = (struct sockaddr*)&bind_in;
        }
        bind_ret = bind(fd, bind_sockaddr_ptr, addr_len);
        if (bind_ret < 0) {
            return;
        }
        connect_ret = fn_ConnectEx(fd, sockaddr_ptr, addr_len, 0, 0, 0, &per_io->ov);
        if (connect_ret == FALSE) {
            error = GetLastError();
            if (error != ERROR_IO_PENDING) {
                return;
            }
            /* 增加引用计数 */
            knet_channel_ref_incref(channel_ref);
            return;
        }
        setsockopt(fd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
    }
    /* 投递一个0长度send请求 */
    result = WSASend(fd, &sbuf, 1, &bytes, flags, &per_io->ov, 0);
    if (result != 0) {
        error = GetLastError();
        if ((error != ERROR_IO_PENDING) && (error != WSAENOTCONN)) {
            knet_channel_ref_close(channel_ref);
        }
        /* 增加引用计数 */
        knet_channel_ref_incref(channel_ref);
        return;
    }
    /* 增加引用计数 */
    knet_channel_ref_incref(channel_ref);
    /* 设置投递标志 */
    flag |= io_type_send;
    knet_channel_ref_set_flag(channel_ref, flag);
}

int knet_impl_event_add(kchannel_ref_t* channel_ref, knet_channel_event_e e) {
    verify(channel_ref);
    if (knet_channel_ref_check_state(channel_ref, channel_state_close)) {
        return error_already_close;
    }
    /* 投递事件 */
    if (channel_event_recv & e) {
        on_iocp_recv(channel_ref);
    } 
    if (e & channel_event_send) {
        on_iocp_send(channel_ref);
    }
    return error_ok;
}

int knet_impl_event_remove(kchannel_ref_t* channel_ref, knet_channel_event_e e) {
    int flag = 0;
    verify(e);
    verify(channel_ref);
    flag = knet_channel_ref_get_flag(channel_ref);
    if (flag & io_type_recv) {
        flag &= ~io_type_recv;
    } 
    if (flag & io_type_send) {
        flag &= ~io_type_send;
    }
    knet_channel_ref_set_flag(channel_ref, flag);
    return error_ok;
}

int knet_impl_add_channel_ref(kloop_t* loop, kchannel_ref_t* channel_ref) {
    loop_iocp_t* impl      = 0;
    socket_t     socket_fd = 0;
    HANDLE       iocp      = 0;
    per_sock_t*  per_sock  = 0;
    verify(loop);
    verify(channel_ref);
    impl      = get_impl(loop);
    socket_fd = knet_channel_ref_get_socket_fd(channel_ref);
    iocp      = 0;
    per_sock  = socket_data_create();
    verify(per_sock);
    per_sock->channel_ref  = channel_ref;
    /* 与IOCP关联 */
    iocp = CreateIoCompletionPort((HANDLE)socket_fd, impl->iocp, (ULONG_PTR)per_sock, 0);
    if (!iocp) {
        socket_data_destroy(per_sock);
        return error_impl_add_channel_ref_fail;
    }
    knet_channel_ref_set_data(channel_ref, per_sock);
    return error_ok;
}

int knet_impl_remove_channel_ref(kloop_t* loop, kchannel_ref_t* channel_ref) {
    per_sock_t* per_sock  = 0;
    verify(loop);
    verify(channel_ref);
    per_sock = get_data(channel_ref);
    verify(per_sock);
    /* 销毁相关资源 */
    socket_data_destroy(per_sock);
    return error_ok;
}

#endif
