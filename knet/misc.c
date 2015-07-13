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

#if !defined(WIN32)
    #include <linux/tcp.h> /* TCP_NODELAY */
#endif /* !defined(WIN32) */

#include "misc.h"
#include "loop.h"
#include "channel_ref.h"
#include "address.h"
#include "timer.h"


struct _thread_runner_t {
    thread_func_t func;
    void* params;
    volatile int running;
    thread_id_t thread_id;
#if defined(WIN32)
    DWORD tls_key;
#else
    pthread_key_t tls_key;
#endif /* defined(WIN32) */
};

socket_t socket_create() {
#if LOOP_IOCP
    socket_t socket_fd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
#else
    socket_t socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif /* LOOP_IOCP */
#if defined(WIN32)
    if (socket_fd == INVALID_SOCKET) {
        log_error("socket() failed, system error: %d", sys_get_errno());
        return 0;
    }
#else
    if (socket_fd < 0) {
        log_error("socket() failed, system error: %d", sys_get_errno());
        return 0;
    }
#endif /* defined(WIN32) */
    return socket_fd;
}

int socket_connect(socket_t socket_fd, const char* ip, int port) {
#if defined(WIN32)
    DWORD last_error = 0;
#endif /* defined(WIN32) || defined(WIN64) */
    int error = 0;
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port   = htons((unsigned short)port);
#if defined(WIN32)
    sa.sin_addr.S_un.S_addr = INADDR_ANY;
    if (ip) {
        sa.sin_addr.S_un.S_addr = inet_addr(ip);
    }
#else
    sa.sin_addr.s_addr = INADDR_ANY;
    if (ip) {
        sa.sin_addr.s_addr = inet_addr(ip);
    }
#endif /* defined(WIN32) || defined(WIN64) */
    error = connect(socket_fd, (struct sockaddr*)&sa, sizeof(struct sockaddr));
#if defined(WIN32)
    if (error < 0) {
        last_error = GetLastError();
        if ((WSAEWOULDBLOCK != last_error) && (WSAEISCONN != last_error)) {
            log_error("connect() failed, system error: %d", sys_get_errno());
            return error_connect_fail;
        }
    }
#else
    if (error < 0) {
        if ((errno != EINPROGRESS) && (errno != EINTR) && (errno != EISCONN)) {
            log_error("connect() failed, system error: %d", sys_get_errno());
            return error_connect_fail;
        }
    }
#endif /* defined(WIN32) || defined(WIN64) */
    return error_ok;
}

int socket_bind_and_listen(socket_t socket_fd, const char* ip, int port, int backlog) {
    int error = 0;
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port   = htons((unsigned short)port);
#if defined(WIN32)
    sa.sin_addr.S_un.S_addr = INADDR_ANY;
    if (ip) {
        sa.sin_addr.S_un.S_addr = inet_addr(ip);
    }
#else
    sa.sin_addr.s_addr = INADDR_ANY;
    if (ip) {
        sa.sin_addr.s_addr = inet_addr(ip);
    }
#endif /* defined(WIN32) || defined(WIN64) */
    socket_set_reuse_addr_on(socket_fd);
    socket_set_linger_off(socket_fd);
    error = bind(socket_fd, (struct sockaddr*)&sa, sizeof(struct sockaddr));
    if (error < 0) {
        log_error("bind() failed, system error: %d", sys_get_errno());
        return error_bind_fail;
    }
    /* 监听 */
    error = listen(socket_fd, backlog);
    if (error < 0) {
        log_error("listen() failed, system error: %d", sys_get_errno());
        return error_listen_fail;
    }
    return error_ok;
}

socket_t socket_accept(socket_t socket_fd) {
    socket_t     client_fd = 0; /* 客户端套接字 */
    socket_len_t addr_len  = sizeof(struct sockaddr_in);
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    /* 接受客户端 */
    client_fd = accept(socket_fd, (struct sockaddr*)&sa, &addr_len);
    if (client_fd < 0) {
        log_error("accept() failed, system error: %d", sys_get_errno());
        return 0;
    }
    return client_fd;
}

int socket_set_reuse_addr_on(socket_t socket_fd) {
    int reuse_addr = 1;
    return setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr , sizeof(reuse_addr));
}

int socket_set_non_blocking_on(socket_t socket_fd) {
#if defined(WIN32)
    u_long nonblocking = 1;
    if (socket_fd == INVALID_SOCKET) {
        verify(0);
        return 1;
    }
    if (SOCKET_ERROR == ioctlsocket(socket_fd, FIONBIO, &nonblocking)) {
        verify(0);
        return 1;
    }
#else
    int flags = 0;
    if (socket_fd < 0) {
        verify(0);
        return 1;
    }
    flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
#endif /* defined(WIN32) */
    return 0;
}

int socket_close(socket_t socket_fd) {
#if defined(WIN32)
    #if LOOP_IOCP
    CancelIo((HANDLE)socket_fd);
    #endif /* LOOP_IOCP */
    return closesocket(socket_fd);
#else
    return close(socket_fd);
#endif /* defined(WIN32) */
}

int socket_set_nagle_off(socket_t socket_fd) {
    int nodelay = 1;
    return setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
}

int socket_set_linger_off(socket_t socket_fd) {
    struct linger linger;
    memset(&linger, 0, sizeof(linger));
    return setsockopt(socket_fd, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));
}

int socket_set_keepalive_off(socket_t socket_fd) {
    int keepalive = 0;
    return setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&keepalive, sizeof(keepalive));
}

int socket_set_donot_route_on(socket_t socket_fd) {
    int donot_route = 1;
    return setsockopt(socket_fd, SOL_SOCKET, SO_DONTROUTE, (char*)&donot_route, sizeof(donot_route));
}

int socket_set_recv_buffer_size(socket_t socket_fd, int size) {
    return setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(size));
}

int socket_set_send_buffer_size(socket_t socket_fd, int size) {
    return setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof(size));
}

int socket_check_send_ready(socket_t socket_fd) {
#if defined(WIN32)
    struct timeval tv = {0, 0};
    int error = 0;
    fd_set send_fds;
    memset(&send_fds, 0, sizeof(fd_set));
    FD_ZERO(&send_fds);
    FD_SET(socket_fd, &send_fds);
    error = select(socket_fd + 1, 0, &send_fds, 0, &tv);
    if (0 > error) {
        return 0;
    }
    return FD_ISSET(socket_fd, &send_fds);
#else
    return 0;
#endif /* defined(WIN32) */    
}

int socket_send(socket_t socket_fd, const char* data, uint32_t size) {
    int send_bytes = 0;
#if defined(WIN32)
    DWORD error = 0;
    send_bytes = send(socket_fd, data, (int)size, 0);
#else
    send_bytes = send(socket_fd, data, (int)size, MSG_NOSIGNAL);
#endif /* defined(WIN32) */
    if (send_bytes < 0) {
    #if defined(WIN32)
        error = GetLastError();
        if ((error == 0) || (error == WSAEINTR) || (error == WSAEINPROGRESS) || (error == WSAEWOULDBLOCK)) {
            return 0;
        } else {
            log_error("send() failed, system error: %d", sys_get_errno());
            send_bytes = -1;
        }
    #else
        if ((errno == 0) || (errno == EAGAIN ) || (errno == EWOULDBLOCK) || (errno == EINTR)) {
            return 0;
        } else {
            log_error("send() failed, system error: %d", sys_get_errno());
            send_bytes = -1;
        }
    #endif /* defined(WIN32) || defined(WIN64) */
    } else if (!send_bytes && size) {
        log_error("send() failed, system error: %d", sys_get_errno());
        return -1;
    }
    return send_bytes;
}

int socket_recv(socket_t socket_fd, char* data, uint32_t size) {
    int recv_bytes = 0;
#if defined(WIN32)
    DWORD error = 0;
    recv_bytes = recv(socket_fd, data, (int)size, 0);
#else
    recv_bytes = recv(socket_fd, data, (int)size, MSG_NOSIGNAL);
#endif /* defined(WIN32) */
    if (recv_bytes < 0) {
    #if defined(WIN32)
        error = GetLastError();
        if ((error == 0) || (error == WSAEINTR) || (error == WSAEINPROGRESS) || (error == WSAEWOULDBLOCK)) {
            return 0;
        } else {
            log_error("recv() failed, system error: %d", sys_get_errno());
            recv_bytes = -1;
        }
    #else
        if ((errno == 0) || (errno == EAGAIN ) || (errno == EWOULDBLOCK) || (errno == EINTR)) {
            return 0;
        } else {
            log_error("recv() failed, system error: %d", sys_get_errno());
            recv_bytes = -1;
        }
    #endif /* defined(WIN32) || defined(WIN64) */
    } else if (recv_bytes == 0) {
        log_error("recv() failed, system error: %d", sys_get_errno());
        recv_bytes = -1;
    }
    return recv_bytes;
}

#if defined(WIN32)
u_short _get_random_port(int begin, int gap) {
    srand((int)time(0));
    return (u_short)(begin + abs(rand() % gap));
}
#endif /* defined(WIN32) || defined(WIN64) */

int socket_pair(socket_t pair[2]) {
#if defined(WIN32)
    int      error       = 1;
    long     flag        = 1;
    int      port_begin  = 20000;
    int      port_gap    = 30000;
    int      addr_len    = sizeof(struct sockaddr_in);
    u_short  port        = _get_random_port(port_begin, port_gap);
    socket_t accept_sock = INVALID_SOCKET;
    struct sockaddr_in accept_addr;
    struct sockaddr_in connect_addr;
    memset(pair, INVALID_SOCKET, sizeof(socket_t) * 2);
    memset(&accept_addr, 0, sizeof(accept_addr));
    memset(&connect_addr, 0, sizeof(connect_addr));
    accept_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (accept_sock == INVALID_SOCKET) {
        log_error("socket() failed, system error: %d", sys_get_errno());
        goto error_return;
    }    
    memset(&accept_addr, 0, sizeof(accept_addr));
    accept_addr.sin_port = htons(port);
    accept_addr.sin_family = AF_INET;
    accept_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    /* 绑定随机端口 */
    error = bind(accept_sock, (struct sockaddr*)&accept_addr,sizeof(accept_addr));
    while (error) {
        if (WSAEADDRINUSE != GetLastError()) {
            log_error("bind() failed, system error: %d", sys_get_errno());
            goto error_return;
        }
        /* 随机分配一个端口 */
        port = _get_random_port(port_begin, port_gap);
        memset(&accept_addr, 0, sizeof(accept_addr));
        accept_addr.sin_port = htons(port);
        accept_addr.sin_family = AF_INET;
        accept_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        /* 重新绑定 */
        error = bind(accept_sock, (struct sockaddr*)&accept_addr,sizeof(accept_addr));
    }
    /* 监听 */
    error = listen(accept_sock, 1);
    if (error) {
        log_error("listen() failed, system error: %d", sys_get_errno());
        goto error_return;
    }
    /* 获取地址 */
    error = getsockname(accept_sock, (struct sockaddr*)&connect_addr, &addr_len);
    if (error) {
        log_error("getsockname() failed, system error: %d", sys_get_errno());
        goto error_return;
    }
    /* 建立客户端套接字 */
    pair[0] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (pair[0] == INVALID_SOCKET) {
        log_error("socket() failed, system error: %d", sys_get_errno());
        goto error_return;
    }
    /* 设置非阻塞 */
    ioctlsocket(pair[0], FIONBIO, (u_long*)&flag);
    connect_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    /* 建立连接 */
    error = connect(pair[0], (struct sockaddr*)&connect_addr, sizeof(connect_addr));
    if(error < 0) {
        error = WSAGetLastError();
        if ((error != WSAEWOULDBLOCK) && (error != WSAEINPROGRESS)) {
            log_error("connect() failed, system error: %d", sys_get_errno());
            goto error_return;
        }
    }
    /* 接受连接 */
    pair[1] = accept(accept_sock, (struct sockaddr*)&accept_addr, &addr_len);
    if(pair[1] == INVALID_SOCKET) {
        log_error("accept() failed, system error: %d", sys_get_errno());
        goto error_return;
    }
    socket_close(accept_sock);
    return 0;

error_return:
    if (accept_sock != INVALID_SOCKET) {
        closesocket(accept_sock);
    }
    if (pair[0] != INVALID_SOCKET) {
        closesocket(pair[0]);
    }
    if (pair[1] != INVALID_SOCKET) {
        closesocket(pair[1]);
    }
    return 1;
#else
    int error = socketpair(AF_UNIX, SOCK_STREAM, 0, pair);
    if (error) {
        log_error("socketpair() failed, system error: %d", sys_get_errno());
        return 1;
    }
    return (socket_set_non_blocking_on(pair[0]) && socket_set_non_blocking_on(pair[1]));
#endif /* defined(WIN32) || defined(WIN64) */
}

int socket_getpeername(channel_ref_t* channel_ref, address_t* address) {
#if defined(WIN32)
    char* ip;
#else
    char ip[32] = {0};
#endif /* defined(WIN32) || define(WIN64) */
    int port;
    struct sockaddr_in addr;
    socket_len_t len = sizeof(struct sockaddr);
    int retval = getpeername(channel_ref_get_socket_fd(channel_ref), (struct sockaddr*)&addr, &len);
    if (retval < 0) {
        log_error("getpeername() failed, system error: %d", sys_get_errno());
        return error_getpeername;
    }
#if defined(WIN32)
    ip = inet_ntoa(addr.sin_addr);
#else
    inet_ntop(AF_INET, &addr.sin_addr.s_addr, ip, sizeof(ip));
#endif /* defined(WIN32) || define(WIN64) */
    port = ntohs(addr.sin_port);
    address_set(address, ip, port);
    return error_ok;
}

int socket_getsockname(channel_ref_t* channel_ref,address_t* address) {
#if defined(WIN32)
    char* ip;
#else
    char ip[32] = {0};
#endif /* defined(WIN32) || define(WIN64) */
    int port;
    struct sockaddr_in addr;
    socket_len_t len = sizeof(struct sockaddr);
    int retval = getsockname(channel_ref_get_socket_fd(channel_ref), (struct sockaddr*)&addr, &len);
    if (retval < 0) {
        log_error("getsockname() failed, system error: %d", sys_get_errno());
        return error_getpeername;
    }
#if defined(WIN32)
    ip = inet_ntoa(addr.sin_addr);
#else
    inet_ntop(AF_INET, &addr.sin_addr.s_addr, ip, sizeof(ip));
#endif /* defined(WIN32) || define(WIN64) */
    port = ntohs(addr.sin_port);
    address_set(address, ip, port);
    return error_ok;
}

atomic_counter_t atomic_counter_inc(atomic_counter_t* counter) {
#if defined(WIN32)
    return InterlockedIncrement(counter);
#else
    return __sync_add_and_fetch(counter, 1);
#endif /* defined(WIN32) */
}

atomic_counter_t atomic_counter_dec(atomic_counter_t* counter) {
#if defined(WIN32)
    return InterlockedDecrement(counter);
#else
    return __sync_sub_and_fetch(counter, 1);
#endif /* defined(WIN32) */
}

int atomic_counter_zero(atomic_counter_t* counter) {
    return (*counter == 0);
}

struct _lock_t {
    #if defined(WIN32)
        CRITICAL_SECTION lock;
    #else
        pthread_mutex_t lock;
    #endif /* defined(WIN32) || defined(WIN64) */
};

void _lock_init(lock_t* lock) {
    #if defined(WIN32)
        InitializeCriticalSection(&lock->lock);
    #else
        pthread_mutex_init(&lock->lock, 0);
    #endif /* defined(WIN32) || defined(WIN64) */ 
}

lock_t* lock_create() {
    lock_t* lock = create(lock_t);
    verify(lock);
    _lock_init(lock);
    return lock;
}

void lock_destroy(lock_t* lock) {
    verify(lock);
    #if defined(WIN32)
        DeleteCriticalSection(&lock->lock);
    #else
        pthread_mutex_destroy(&lock->lock);
    #endif /* defined(WIN32) || defined(WIN64) */
    destroy(lock);
}

void lock_lock(lock_t* lock) {
    verify(lock);
    #if defined(WIN32)
        EnterCriticalSection(&lock->lock);
    #else
        pthread_mutex_lock(&lock->lock);
    #endif /* defined(WIN32) || defined(WIN64) */ 
}

int lock_trylock(lock_t* lock) {
    verify(lock);
    #if defined(WIN32)
        return TryEnterCriticalSection(&lock->lock);
    #else
        return !pthread_mutex_trylock(&lock->lock);
    #endif /* defined(WIN32) || defined(WIN64) */ 
}

void lock_unlock(lock_t* lock) {
    verify(lock);
    #if defined(WIN32)
        LeaveCriticalSection(&lock->lock);
    #else
        pthread_mutex_unlock(&lock->lock);
    #endif /* defined(WIN32) || defined(WIN64) */ 
}

thread_runner_t* thread_runner_create(thread_func_t func, void* params) {
    thread_runner_t* runner = create(thread_runner_t);
    verify(runner);
    memset(runner, 0, sizeof(thread_runner_t));
    runner->func   = func;
    runner->params = params;
    return runner;
}

void thread_runner_destroy(thread_runner_t* runner) {
    verify(runner);
    if (runner->running) {
        thread_runner_stop(runner);
    }
    thread_runner_join(runner);
    if (runner->tls_key) {
#if defined(WIN32)
        TlsFree(runner->tls_key);
#else
        pthread_key_delete(runner->tls_key);
#endif /* defined(WIN32) */
    }
    destroy(runner);
}

void _thread_func(void* params) {
    thread_runner_t* runner = 0;
    verify(params);
    runner = (thread_runner_t*)params;
    runner->func(runner->params);
}

void _thread_loop_func(void* params) {
    int error = 0;
    thread_runner_t* runner = (thread_runner_t*)params;
    loop_t* loop = (loop_t*)runner->params;
    while (thread_runner_check_start(runner)) {
        error = loop_run_once(loop);
        if (error != error_ok) {
            thread_runner_stop(runner);
            verify(0);
        }
    }
}

void _thread_timer_loop_func(void* params) {
    thread_runner_t* runner = (thread_runner_t*)params;
    ktimer_loop_t* loop = (ktimer_loop_t*)runner->params;
    int tick = (int)ktimer_loop_get_tick_intval(loop);
    while (thread_runner_check_start(runner)) {
        thread_sleep_ms(tick);
        ktimer_loop_run_once(loop);
    }
}

#if defined(WIN32)
void thread_loop_func_win(void* params) {
    _thread_loop_func(params);
}
#else
void* thread_loop_func_pthread(void* params) {
    _thread_loop_func(params);
    return 0;
}
#endif /* defined(WIN32) || defined(WIN64) */

#if defined(WIN32)
void thread_timer_loop_func_win(void* params) {
    _thread_timer_loop_func(params);
}
#else
void* thread_timer_loop_func_pthread(void* params) {
    _thread_timer_loop_func(params);
    return 0;
}
#endif /* defined(WIN32) || defined(WIN64) */

#if defined(WIN32)
void thread_func_win(void* params) {
    _thread_func(params);
}
#else
void* thread_func_pthread(void* params) {
    _thread_func(params);
    return 0;
}
#endif /* defined(WIN32) || defined(WIN64) */

int thread_runner_start(thread_runner_t* runner, int stack_size) {
#if defined(WIN32)
    uintptr_t retval = 0;
#else
    int retval = 0;
    pthread_attr_t attr;
#endif /* defined(WIN32) || defined(WIN64) */
    verify(runner);
    if (!runner->func) {
        return error_thread_start_fail;
    }
    runner->running = 1;
#if defined(WIN32)
    retval = _beginthread(thread_func_win, stack_size, runner);
    if (retval < 0) {
        log_error("_beginthread() failed, system error: %d", sys_get_errno());
        return error_thread_start_fail;
    }
    runner->thread_id = retval;
#else
    if (stack_size) {
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, stack_size);
        retval = pthread_create(&runner->thread_id, &attr, thread_func_pthread, runner);
    } else {
        retval = pthread_create(&runner->thread_id, 0, thread_func_pthread, runner);
    }
    if (retval) {
        log_error("pthread_create() failed, system error: %d", sys_get_errno());
        return error_thread_start_fail;
    }
#endif /* defined(WIN32) || defined(WIN64) */
    return error_ok;
}

int thread_runner_start_loop(thread_runner_t* runner, loop_t* loop, int stack_size) {
#if defined(WIN32)
    uintptr_t retval = 0;
#else
    int retval = 0;
    pthread_attr_t attr;
#endif /* defined(WIN32) || defined(WIN64) */
    verify(runner);
    verify(loop);
    runner->params = loop;
    runner->running = 1;
#if defined(WIN32)
    retval = _beginthread(thread_loop_func_win, stack_size, runner);
    if (retval < 0) {
        log_error("_beginthread() failed, system error: %d", sys_get_errno());
        return error_thread_start_fail;
    }
    runner->thread_id = retval;
#else
    if (stack_size) {
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, stack_size);
        retval = pthread_create(&runner->thread_id, &attr, thread_loop_func_pthread, runner);
    } else {
        retval = pthread_create(&runner->thread_id, 0, thread_loop_func_pthread, runner);
    }
    if (retval) {
        log_error("pthread_create() failed, system error: %d", sys_get_errno());
        return error_thread_start_fail;
    }
#endif /* defined(WIN32) || defined(WIN64) */
    return error_ok;
}

int thread_runner_start_timer_loop(thread_runner_t* runner, ktimer_loop_t* timer_loop, int stack_size) {
#if defined(WIN32)
    uintptr_t retval = 0;
#else
    int retval = 0;
    pthread_attr_t attr;
#endif /* defined(WIN32) || defined(WIN64) */
    verify(runner);
    verify(timer_loop);
    runner->params = timer_loop;
    runner->running = 1;
#if defined(WIN32)
    retval = _beginthread(thread_timer_loop_func_win, stack_size, runner);
    if (retval < 0) {
        log_error("_beginthread() failed, system error: %d", sys_get_errno());
        return error_thread_start_fail;
    }
    runner->thread_id = retval;
#else
    if (stack_size) {
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, stack_size);
        retval = pthread_create(&runner->thread_id, &attr, thread_timer_loop_func_pthread, runner);
    } else {
        retval = pthread_create(&runner->thread_id, 0, thread_timer_loop_func_pthread, runner);
    }
    if (retval) {
        log_error("pthread_create() failed, system error: %d", sys_get_errno());
        return error_thread_start_fail;
    }
#endif /* defined(WIN32) || defined(WIN64) */
    return error_ok;
}

void thread_runner_stop(thread_runner_t* runner) {
    verify(runner);
    runner->running = 0;
}

void thread_runner_join(thread_runner_t* runner) {
#if defined(WIN32)
    DWORD error = 0;
#else
    void* retval = 0;
#endif /* defined(WIN32) || defined(WIN64) */
    verify(runner);
    /* 建立但未启动 */
    if (!runner->thread_id) {
        return;
    }
#if defined(WIN32)
    error = WaitForSingleObject((HANDLE)runner->thread_id, INFINITE);
    if ((error != WAIT_OBJECT_0) && (error != WAIT_ABANDONED)) {
        log_error("WaitForSingleObject() failed, system error: %d", sys_get_errno());
    }
#else
    pthread_join(runner->thread_id, &retval);
#endif /* defined(WIN32) || defined(WIN64) */
}

int thread_runner_check_start(thread_runner_t* runner) {
    verify(runner);
    return runner->running;
}

void* thread_runner_get_params(thread_runner_t* runner) {
    verify(runner);
    return runner->params;
}

thread_id_t thread_get_self_id() {
#if defined(WIN32)
    return GetCurrentThreadId();
#else
    return pthread_self();
#endif /* defined(WIN32) */
}

void thread_sleep_ms(int ms) {
#if defined(WIN32)
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif /* defined(WIN32) */
}

int thread_set_tls_data(thread_runner_t* runner, void* data) {
    verify(runner);
    if (!runner->tls_key) {
#if defined(WIN32)
        runner->tls_key = TlsAlloc();
        if (runner->tls_key == TLS_OUT_OF_INDEXES) {
            log_error("TlsAlloc() failed, system error: %d", sys_get_errno());
            return error_set_tls_fail;
        }
#else
        if (pthread_key_create(&runner->tls_key, 0)) {
            log_error("pthread_key_create() failed, system error: %d", sys_get_errno());
            return error_set_tls_fail;
        }
#endif /* defined(WIN32) */
    }
#if defined(WIN32)
    if (FALSE == TlsSetValue(runner->tls_key, data)) {
        log_error("TlsSetValue() failed, system error: %d", sys_get_errno());
        return error_set_tls_fail;
    }
#else
    if (pthread_setspecific(runner->tls_key, data)) {
        log_error("pthread_setspecific() failed, system error: %d", sys_get_errno());
        return error_set_tls_fail;
    }
#endif /* defined(WIN32) */
    return error_ok;
}

void* thread_get_tls_data(thread_runner_t* runner) {
    verify(runner);
    if (!runner->tls_key) {
        log_error("tls never initialize");
        return 0;
    }
#if defined(WIN32)
    return TlsGetValue(runner->tls_key);
#else
    return pthread_getspecific(runner->tls_key);
#endif /* defined(WIN32) */
}

uint32_t time_get_milliseconds() {
#if defined(WIN32)
    return GetTickCount();
#else
    struct timeval tv;
    uint64_t ms;
    gettimeofday(&tv, 0);
    ms = tv.tv_sec * 1000;
    ms += tv.tv_usec / 1000;
    return ms;
#endif /* defined(WIN32) */
}

int time_gettimeofday(struct timeval *tp, void *tzp) {
#if defined(WIN32)
    time_t clock;
    struct tm tm;
    SYSTEMTIME st;
    tzp;
    GetLocalTime(&st);
    tm.tm_year  = st.wYear - 1900;
    tm.tm_mon   = st.wMonth - 1;
    tm.tm_mday  = st.wDay;
    tm.tm_hour  = st.wHour;
    tm.tm_min   = st.wMinute;
    tm.tm_sec   = st.wSecond;
    tm.tm_isdst = -1;
    clock = mktime(&tm);
    tp->tv_sec = (long)clock;
    tp->tv_usec = st.wMilliseconds * 1000;
    return 0;
#else
    return gettimeofday(tp, tzp);
#endif /* defined(WIN32) */
}

char* time_get_string(char* buffer, int size) {
    struct timeval tp;
    struct tm      t;
    time_t timestamp = time(0);
    verify(buffer);
    verify(size);
    time_gettimeofday(&tp, 0);
    memset(buffer, 0, size);
#if defined(WIN32)
    localtime_s(&t, &timestamp);
    _snprintf(buffer, size, "%4d-%02d-%02d %02d:%02d:%02d:%03d",
        t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
            t.tm_hour, t.tm_min, t.tm_sec, (int)(tp.tv_usec / 1000));
#else
    localtime_r(&timestamp, &t);
    snprintf(buffer, size, "%4d-%02d-%02d %02d:%02d:%02d:%03d",
        t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
            t.tm_hour, t.tm_min, t.tm_sec, (int)(tp.tv_usec / 1000));
#endif /* defined(WIN32) */
    buffer[size - 1] = 0;
    return buffer;
}

uint64_t time_get_microseconds() {
#if defined(WIN32)
    LARGE_INTEGER freq;
    LARGE_INTEGER fc;
    if (!QueryPerformanceFrequency(&freq)) {
        verify(0);
    }
    if (!QueryPerformanceCounter(&fc)) {
        verify(0);
    }
    return fc.QuadPart / (freq.QuadPart / 1000 / 1000);
#else
    struct timeval tv;
    uint64_t ms;
    gettimeofday(&tv, 0);
    ms = tv.tv_sec * 1000 * 1000;
    ms += tv.tv_usec;
    return ms;
#endif /* defined(WIN32) || defined(WIN64) */
}

uint64_t uuid_create() {
    static atomic_counter_t fack_uuid_low  = 0;
    static atomic_counter_t fack_uuid_high = 1;
    uint64_t uuid = 0;
    if (!fack_uuid_low) {
        fack_uuid_low = (uint32_t)time_get_microseconds();
    }
    atomic_counter_inc(&fack_uuid_high);
    uuid = fack_uuid_high; /* 高32位 */
    uuid <<= 32;
    uuid += fack_uuid_low; /* 低32位 */
    return uuid;
}

char* path_getcwd(char* buffer, int size) {
#if defined(WIN32)
    if (!GetCurrentDirectoryA(size, buffer)) {
        log_error("GetCurrentDirectoryA() failed, system error: %d", sys_get_errno());
        return 0;
    }
#else
    if (!getcwd(buffer, size)) {
        log_error("getcwd() failed, system error: %d", sys_get_errno());
        return 0;
    }
#endif /* defined(WIN32) */
    return buffer;
}

sys_error_t sys_get_errno() {
#if defined(WIN32)
    return GetLastError();
#else
    return errno;
#endif /* defined(WIN32) */
}

uint64_t htonll(uint64_t ui64) { 
     return (((uint64_t)htonl((uint32_t)ui64)) << 32) + htonl(ui64 >> 32); 
} 

uint64_t ntohll(uint64_t ui64) { 
     return (((uint64_t)ntohl((uint32_t)ui64)) << 32) + ntohl(ui64 >> 32); 
} 

const char* get_channel_cb_event_string(channel_cb_event_e e) {
    switch (e) {
    case channel_cb_event_connect:
        return "channel as connector connected successfully";
    case channel_cb_event_accept:
        return "channel as acceptor accepted a new TCP socket";
    case channel_cb_event_recv:
        return "received serveral bytes";
    case channel_cb_event_send:
        return "several bytes has been sent";
    case channel_cb_event_close:
        return "channel has beed closed";
    case channel_cb_event_timeout:
        return "channel idle timeout because there is no bytes received according to the idle timeout setting";
    case channel_cb_event_connect_timeout:
        return "channel try to connect remote host failed because the connect timeout setting reached";
    }
    return "unknown channel callback event";
}

const char* get_channel_cb_event_name(channel_cb_event_e e) {
    switch (e) {
    case channel_cb_event_connect:
        return "channel_cb_event_connect";
    case channel_cb_event_accept:
        return "channel_cb_event_accept";
    case channel_cb_event_recv:
        return "channel_cb_event_recv";
    case channel_cb_event_send:
        return "channel_cb_event_send";
    case channel_cb_event_close:
        return "channel_cb_event_close";
    case channel_cb_event_timeout:
        return "channel_cb_event_timeout";
    case channel_cb_event_connect_timeout:
        return "channel_cb_event_connect_timeout";
    }
    return "unknown channel callback event";
}
