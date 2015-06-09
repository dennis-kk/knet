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

#ifndef LIST_H
#define LIST_H

#include "config.h"

dlist_node_t* dlist_node_create();
dlist_node_t* dlist_node_init(dlist_node_t* node);
void dlist_node_destroy(dlist_node_t* node);
dlist_node_t* dlist_node_set_data(dlist_node_t* node, void* data);
void* dlist_node_get_data(dlist_node_t* node);

dlist_t* dlist_create();
dlist_t* dlist_init(dlist_t* dlist);
void dlist_destroy(dlist_t* dlist);
void dlist_add_front(dlist_t* dlist, dlist_node_t* node);
void dlist_add_tail(dlist_t* dlist, dlist_node_t* node);
dlist_node_t* dlist_add_front_node(dlist_t* dlist, void* data);
dlist_node_t* dlist_add_tail_node(dlist_t* dlist, void* data);
int dlist_get_count(dlist_t* dlist);
int dlist_empty(dlist_t* dlist);
dlist_node_t* dlist_remove(dlist_t* dlist, dlist_node_t* node);
void dlist_delete(dlist_t* dlist, dlist_node_t* node);
dlist_node_t* dlist_next(dlist_t* dlist, dlist_node_t* node);
dlist_node_t* dlist_get_front(dlist_t* dlist);
dlist_node_t* dlist_get_back(dlist_t* dlist);

/* 遍历链表，在遍历的同时不能删除或销毁链表节点 */
#define dlist_for_each(list, node) \
    for (node = dlist_get_front(list); (node); node = dlist_next(list, node))

/* 遍历链表，可以在遍历同时进行删除或销毁链表节点 */
#define dlist_for_each_safe(list, node, temp) \
    for (node = dlist_get_front(list), temp = dlist_next(list, node); (node); node = temp, temp = dlist_next(list, node))

#endif /* LIST_H */
