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

#include <ctype.h>
#include "ip_filter_api.h"
#include "trie_api.h"
#include "address.h"

struct _ip_filter_t {
    ktrie_t* trie; /* IP trie */
};

/**
 * 去除字符串起始和结束的空白
 * @param ip IP
 * @param size 字符串长度
 * @return 去除空白后字符串新的起止地址
 */
char* _trim(char* ip, int size);

/**
 * 遍历函数
 */
int _ip_filter_for_each_func(const char* ip, void* param);

int _ip_filter_for_each_func(const char* ip, void* param) {
    FILE* fp = (FILE*)param;
    verify(ip);
    verify(param);
    verify(fp);
    if (0 >= fprintf(fp, "%s\n", ip)) {
        return 1;
    }
    return 0;
}

char* _trim(char* ip, int size) {
    char* ptr = 0;
    int   i   = 0;
    /* 左侧 */
    for (; i < size; i++) {
        if (isdigit(ip[i]) || !ip[i]) {
            break;
        }
    }
    ptr = ip + i;
    if (*ptr) {
        /* 右侧 */
        i = size - 1;
        for (; i >= 0; i--) {
            if (!isdigit(ip[i])) {
                ip[i] = 0;
            } else {
                break;
            }
        }
    }
    return ptr;
}

kip_filter_t* knet_ip_filter_create() {
    kip_filter_t* filter = create(kip_filter_t);
    verify(filter);
    memset(filter, 0, sizeof(kip_filter_t));
    filter->trie = trie_create();
    verify(filter->trie);
    return filter;
}

void knet_ip_filter_destroy(kip_filter_t* ip_filter) {
    verify(ip_filter);
    trie_destroy(ip_filter->trie, 0);
    destroy(ip_filter);
}

int knet_ip_filter_load_file(kip_filter_t* ip_filter, const char* path) {
    char  ip[32] = {0};
    char* ptr    = 0;
    FILE* fp     = 0;
    int   error  = error_ok;
    verify(ip_filter);
    verify(path);
    fp = fopen(path, "r+");
    if (!fp) {
        error = error_ip_filter_open_fail;
        goto error_return;
    }
    while (fgets(ip, sizeof(ip), fp)) {
        ptr = _trim(ip, sizeof(ip));
        if (ptr[0]) {
            error = trie_insert(ip_filter->trie, ip, 0);
            if (error_ok != error) {
                goto error_return;
            }
        }
        memset(ip, 0, sizeof(ip));
    }
    fclose(fp);
    return error;
error_return:
    if (fp) {
        fclose(fp);
    }
    return error;
}

int knet_ip_filter_add(kip_filter_t* ip_filter, const char* ip) {
    verify(ip_filter);
    verify(ip);
    return trie_insert(ip_filter->trie, ip, 0);
}

int knet_ip_filter_remove(kip_filter_t* ip_filter, const char* ip) {
    verify(ip_filter);
    verify(ip);
    return trie_remove(ip_filter->trie, ip, 0);
}

int knet_ip_filter_save(kip_filter_t* ip_filter, const char* path) {
    int   error = error_ok;
    FILE* fp    = 0;
    verify(ip_filter);
    verify(path);
    fp = fopen(path, "w+");
    if (!fp) {
        return error_ip_filter_open_fail;
    }
    error = trie_for_each(ip_filter->trie, _ip_filter_for_each_func, fp);
    fclose(fp);
    return error;
}

int knet_ip_filter_check(kip_filter_t* ip_filter, const char* ip) {
    verify(ip_filter);
    verify(ip);
    return (trie_find(ip_filter->trie, ip, 0) == error_ok ? 1 : 0);
}

int knet_ip_filter_check_address(kip_filter_t* ip_filter, kaddress_t* address) {
    verify(ip_filter);
    verify(address);
    return knet_ip_filter_check(ip_filter, address_get_ip(address));
}
