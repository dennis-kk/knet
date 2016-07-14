/*
 * Copyright (c) 2014-2016, dennis wang
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

#include "address.h"
#include "logger.h"

/**
 * 地址
 */
struct _address_t {
    char ip[32]; /* IP */
    int  port;   /* 端口 */
};

kaddress_t* knet_address_create() {
    kaddress_t* address = create(kaddress_t);
    verify(address);
    memset(address, 0, sizeof(kaddress_t));
    /* 默认地址 */
    strcpy(address->ip, "0.0.0.0");
    return address;
}

void knet_address_destroy(kaddress_t* address) {
    verify(address);
    if (address) {
        destroy(address);
    }
}

void knet_address_set(kaddress_t* address, const char* ip, int port) {
    verify(address);
    /* 设置IP, 端口 */
    if (ip) {
        strcpy(address->ip, ip);
    }
    address->port = port;
}

const char* address_get_ip(kaddress_t* address) {
    verify(address);
    return address->ip;
}

int address_get_port(kaddress_t* address) {
    verify(address);
    return address->port;
}

int address_equal(kaddress_t* address, const char* ip, int port) {
    verify(address);
    verify(ip);
    verify(port);
    /* 完全相同 */
    return (strcmp(address->ip, ip) || !(address->port == port));
}
