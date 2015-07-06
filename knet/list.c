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

#include <stdlib.h>
#include "list.h"
#include "misc.h"


/* 链表节点 */
struct _dlist_node_t {
    struct _dlist_node_t* prev;
    struct _dlist_node_t* next;
    void*                 data;
    int                   init;
};

/* 双向循环链表 */
struct _dlist_t {
    dlist_node_t*    head;
    atomic_counter_t count;
    int              init;
};

dlist_node_t* dlist_node_create() {
    dlist_node_t* node = create(dlist_node_t);
    verify(node);
    node->next  = 0;
    node->prev  = 0;
    node->data  = 0;
    node->init  = 0;
    return node;
}

dlist_node_t* dlist_node_init(dlist_node_t* node) {
    verify(node);
    node->next  = 0;
    node->prev  = 0;
    node->data  = 0;
    node->init  = 1;
    return node;
}

void dlist_node_destroy(dlist_node_t* node) {
    verify(node);
    /* data在外部销毁 */
    if (!node->init) {
        destroy(node);
    }
}

dlist_node_t* dlist_node_set_data(dlist_node_t* node, void* data) {
    verify(node);
    node->data = data;
    return node;
}

void* dlist_node_get_data(dlist_node_t* node) {
    verify(node);
    return node->data;
}

dlist_t* dlist_create() {
    dlist_t* dlist = create(dlist_t);
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
    dlist->count = 0;
    dlist->init = 0;
    return dlist;
}

dlist_t* dlist_init(dlist_t* dlist) {
    verify(dlist);
    dlist->head = dlist_node_create();
    verify(dlist->head);
    dlist->head->data = 0;
    dlist->head->next = dlist->head;
    dlist->head->prev = dlist->head;
    dlist->count = 0;
    dlist->init = 1;
    return dlist;
}

void dlist_destroy(dlist_t* dlist) {
    dlist_node_t* node = 0;
    dlist_node_t* temp = 0;
    verify(dlist);
    dlist_for_each_safe(dlist, node, temp) {
        dlist_delete(dlist, node);
    }
    destroy(dlist->head);
    if (!dlist->init) {
        destroy(dlist);
    }
}

void dlist_add_front(dlist_t* dlist, dlist_node_t* node) {
    verify(dlist);
    verify(node);
    dlist->head->next->prev = node;
    node->prev = dlist->head;
    node->next = dlist->head->next;
    dlist->head->next = node;
    atomic_counter_inc(&dlist->count);
}

void dlist_add_tail(dlist_t* dlist, dlist_node_t* node) {
    verify(dlist);
    verify(node);
    dlist->head->prev->next = node;
    node->next = dlist->head;
    node->prev = dlist->head->prev;
    dlist->head->prev = node;
    atomic_counter_inc(&dlist->count);
}

dlist_node_t* dlist_add_front_node(dlist_t* dlist, void* data) {
    dlist_node_t* node = 0;
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

dlist_node_t* dlist_add_tail_node(dlist_t* dlist, void* data) {
    dlist_node_t* node = 0;
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

int dlist_get_count(dlist_t* dlist) {
    verify(dlist);
    return dlist->count;
}

int dlist_empty(dlist_t* dlist) {
    verify(dlist);
    return atomic_counter_zero(&dlist->count);
}

dlist_node_t* dlist_remove(dlist_t* dlist, dlist_node_t* node) {
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

void dlist_delete(dlist_t* dlist, dlist_node_t* node) {
    verify(dlist);
    verify(node);
    dlist_node_destroy(dlist_remove(dlist, node));
}

dlist_node_t* dlist_next(dlist_t* dlist, dlist_node_t* node) {
    verify(dlist);
    if (!node) {
        return 0;
    }
    if (dlist->head == node->next) {
        return 0;
    }
    return node->next;
}

dlist_node_t* dlist_get_front(dlist_t* dlist) {
    verify(dlist);
    if (dlist->head->next == dlist->head) {
        return 0;
    }
    return dlist->head->next;
}

dlist_node_t* dlist_get_back(dlist_t* dlist) {
    verify(dlist);
    if (dlist->head->prev == dlist->head) {
        return 0;
    }
    return dlist->head->prev;
}
