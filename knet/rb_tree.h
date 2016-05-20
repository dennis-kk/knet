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
