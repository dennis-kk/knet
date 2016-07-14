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

#include <stdlib.h>
#include "list.h"
#include "misc.h"
#include "logger.h"


/* 链表节点 */
struct _dlist_node_t {
    struct _dlist_node_t* prev; /* 上一个节点 */
    struct _dlist_node_t* next; /* 下一个节点 */
    void*                 data; /* 用户数据指针 */
    int                   init; /* 是否通过调用dlist_node_init初始化 */
};

/* 双向循环链表 */
struct _dlist_t {
    kdlist_node_t*   head;  /* 链表头 */
    atomic_counter_t count; /* 链表内节点数量 */
    int              init;  /* 是否通过调用dlist_init初始化 */
};

kdlist_node_t* dlist_node_create() {
    kdlist_node_t* node = create(kdlist_node_t);
    verify(node);
    node->next = 0;
    node->prev = 0;
    node->data = 0;
    node->init = 0;
    return node;
}

kdlist_node_t* dlist_node_init(kdlist_node_t* node) {
    verify(node);
    node->next = 0;
    node->prev = 0;
    node->data = 0;
    node->init = 1;
    return node;
}

void dlist_node_destroy(kdlist_node_t* node) {
    verify(node);
    /* data在外部销毁 */
    if (!node->init) {
        destroy(node);
    }
}

kdlist_node_t* dlist_node_set_data(kdlist_node_t* node, void* data) {
    verify(node);
    node->data = data;
    return node;
}

void* dlist_node_get_data(kdlist_node_t* node) {
    verify(node);
    return node->data;
}

kdlist_t* dlist_create() {
    kdlist_t* dlist = create(kdlist_t);
    verify(dlist);
    dlist->head = dlist_node_create();
    verify(dlist->head);
    if (!dlist->head) {
        destroy(dlist);
        return 0;
    }
    dlist->head->data = 0;
    dlist->head->next = dlist->head;
    dlist->head->prev = dlist->head;
    dlist->count      = 0;
    dlist->init       = 0;
    return dlist;
}

kdlist_t* dlist_init(kdlist_t* dlist) {
    verify(dlist);
    dlist->head = dlist_node_create();
    verify(dlist->head);
    dlist->head->data = 0;
    dlist->head->next = dlist->head;
    dlist->head->prev = dlist->head;
    dlist->count      = 0;
    dlist->init       = 1;
    return dlist;
}

void dlist_destroy(kdlist_t* dlist) {
    kdlist_node_t* node = 0;
    kdlist_node_t* temp = 0;
    verify(dlist);
    dlist_for_each_safe(dlist, node, temp) {
        dlist_delete(dlist, node);
    }
    destroy(dlist->head);
    if (!dlist->init) {
        destroy(dlist);
    }
}

void dlist_add_front(kdlist_t* dlist, kdlist_node_t* node) {
    verify(dlist);
    verify(node);
    dlist->head->next->prev = node;
    node->prev              = dlist->head;
    node->next              = dlist->head->next;
    dlist->head->next       = node;
    atomic_counter_inc(&dlist->count);
}

void dlist_add_tail(kdlist_t* dlist, kdlist_node_t* node) {
    verify(dlist);
    verify(node);
    dlist->head->prev->next = node;
    node->next              = dlist->head;
    node->prev              = dlist->head->prev;
    dlist->head->prev       = node;
    atomic_counter_inc(&dlist->count);
}

kdlist_node_t* dlist_add_front_node(kdlist_t* dlist, void* data) {
    kdlist_node_t* node = 0;
    verify(dlist);
    node = dlist_node_create();
    verify(node);
    if (!node) {
        return 0;
    }
    dlist_node_set_data(node, data);
    dlist_add_front(dlist, node);
    return node;
}

kdlist_node_t* dlist_add_tail_node(kdlist_t* dlist, void* data) {
    kdlist_node_t* node = 0;
    verify(dlist);
    node = dlist_node_create();
    verify(node);
    if (!node) {
        return 0;
    }
    dlist_node_set_data(node, data);
    dlist_add_tail(dlist, node);
    return node;
}

int dlist_get_count(kdlist_t* dlist) {
    verify(dlist);
    return dlist->count;
}

int dlist_empty(kdlist_t* dlist) {
    verify(dlist);
    return atomic_counter_zero(&dlist->count);
}

kdlist_node_t* dlist_remove(kdlist_t* dlist, kdlist_node_t* node) {
    verify(dlist);
    verify(node);
    if (!node->prev || !node->next) {
        return 0;
    }
    node->prev->next = node->next;
    node->next->prev = node->prev;
    atomic_counter_dec(&dlist->count);
    return node;
}

void dlist_delete(kdlist_t* dlist, kdlist_node_t* node) {
    verify(dlist);
    verify(node);
    dlist_node_destroy(dlist_remove(dlist, node));
}

kdlist_node_t* dlist_next(kdlist_t* dlist, kdlist_node_t* node) {
    verify(dlist);
    if (!node) {
        return 0;
    }
    if (dlist->head == node->next) {
        return 0;
    }
    return node->next;
}

kdlist_node_t* dlist_get_front(kdlist_t* dlist) {
    verify(dlist);
    if (dlist->head->next == dlist->head) {
        return 0;
    }
    return dlist->head->next;
}

kdlist_node_t* dlist_get_back(kdlist_t* dlist) {
    verify(dlist);
    if (dlist->head->prev == dlist->head) {
        return 0;
    }
    return dlist->head->prev;
}
