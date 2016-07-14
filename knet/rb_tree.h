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

#ifndef RB_TREE_H
#define RB_TREE_H

#include "config.h"

/**
 * 建立红黑树节点
 * @param key 节点键
 * @param ptr 节点用户数据指针
 * @param cb 销毁回调函数指针
 * @return 新建立的节点
 */
krbnode_t* krbnode_create(uint64_t key, void* ptr, knet_rb_node_destroy_cb_t cb);

/**
 * 销毁红黑树节点
 * @param node 节点
 */
void krbnode_destroy(krbnode_t* node);

/**
 * 获取节点键
 * @param node 节点
 * @return 键
 */
uint64_t krbnode_get_key(krbnode_t* node);

/**
 * 获取节点用户数据指针
 * @param node 节点
 * @return 用户数据指针
 */
void* krbnode_get_ptr(krbnode_t* node);

/**
 * 建立红黑树
 * @return 红黑树
 */
krbtree_t* krbtree_create();

/**
 * 销毁红黑树
 * @param tree 红黑树
 */
void krbtree_destroy(krbtree_t* tree);

/**
 * 插入红黑树节点
 * @param tree 红黑树
 * @param node 节点
 */
void krbtree_insert(krbtree_t* tree, krbnode_t* node);

/**
 * 查找节点
 * @param tree 红黑树
 * @param key 键
 * @return 节点
 */
krbnode_t* krbtree_find(krbtree_t* tree, uint64_t key);

/**
 * 销毁节点
 * @param tree 红黑树
 * @param node 节点
 */
void krbtree_delete(krbtree_t* tree, krbnode_t* node);

/**
 * 查找最小键的节点
 * @param tree 红黑树
 * @return 节点
 */
krbnode_t* krbtree_min(krbtree_t* tree);

/**
 * 查找最大键的节点
 * @param tree 红黑树
 * @return 节点
 */
krbnode_t* krbtree_max(krbtree_t* tree);

#endif /* RB_TREE_H */
