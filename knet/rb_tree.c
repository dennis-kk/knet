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

#include "config.h"
#include "rb_tree.h"
#include "logger.h"

/*
 * 实现基于算法导论关于红黑树的相关章节
 */

struct _rb_node_t {
    struct _rb_node_t*        parent; /* 父节点 */
    struct _rb_node_t*        left;   /* 左孩子 */
    struct _rb_node_t*        right;  /* 右孩子 */
    uint64_t                  key;    /* 节点键 */
    void*                     ptr;    /* 节点用户数据指针 */
    rb_color_e                color;  /* 节点颜色 */
    knet_rb_node_destroy_cb_t cb;     /* 销毁函数指针 */
};

struct _rb_tree_t {
    krbnode_t* root; /* 根节点 */
};

/*
 * 实现基于算法导论关于红黑树的相关章节
 */

/* 外围节点 */
static krbnode_t nil_node = { 0, 0, 0, 0, 0, rb_color_black };
static krbnode_t* nil = &nil_node;

/**
 * 销毁节点及其所有子节点
 * @param node 节点
 */
void krbnode_destroy_recursive(krbnode_t* node);

/**
 * 对x节点左旋转
 * @param tree 红黑树
 * @param x 节点
 */
void krbtree_left_rotate(krbtree_t* tree, krbnode_t* x);

/**
 * 对x节点右旋转
 * @param tree 红黑树
 * @param x 节点
 */
void krbtree_right_rotate(krbtree_t* tree, krbnode_t* x);

/**
 * 根据二叉搜索树的规则插入节点后修正树,使其符合红黑树的规则
 * @param tree 红黑树
 * @param x 新插入的节点
 */
void krbtree_insert_fixup(krbtree_t* tree, krbnode_t* x);

/**
 * 根据二叉搜索树的规则删除节点后修正树,使其符合红黑树的规则
 * @param tree 红黑树
 * @param x 需要删除的节点
 */
void krbtree_delete_fixup(krbtree_t* tree, krbnode_t* x);

/**
 * 查找某个子树内节点
 * @param tree 子树根节点
 * @param key 键
 * @return 节点
 */
krbnode_t* krbnode_find(krbnode_t* tree, uint64_t key);

/**
 * 获取x的后继节点
 * @param x 节点
 * @return 节点
 */
krbnode_t* krbnode_get_successor(krbnode_t* x);

krbnode_t* krbnode_create(uint64_t key, void* ptr, knet_rb_node_destroy_cb_t cb) {
    krbnode_t* node = create(krbnode_t);
    verify(node);
    memset(node, 0, sizeof(krbnode_t));
    node->key = key;
    node->ptr = ptr;
    node->cb  = cb;
    node->left = nil;
    node->right = nil;
    node->parent = nil;
    return node;
}

void krbnode_destroy(krbnode_t* node) {
    verify(node);
    if (node == nil) {
        return;
    }
    if (node->cb) {
        node->cb(node->ptr, node->key);
    }
    destroy(node);
}

void krbnode_destroy_recursive(krbnode_t* node) {
    if (node->left) {
        krbnode_destroy_recursive(node->left);
    }
    if (node->right) {
        krbnode_destroy_recursive(node->right);
    }
    krbnode_destroy(node);
}

uint64_t krbnode_get_key(krbnode_t* node) {
    verify(node);
    return node->key;
}

void* krbnode_get_ptr(krbnode_t* node) {
    verify(node);
    return node->ptr;
}

rb_color_e krbnode_get_color(krbnode_t* node) {
    verify(node);
    return node->color;
}

krbtree_t* krbtree_create() {
    krbtree_t* tree = create(krbtree_t);
    verify(tree);
    memset(tree, 0, sizeof(krbtree_t));
    return tree;
}

void krbtree_destroy(krbtree_t* tree) {
    krbnode_t* node = 0;
    verify(tree);
    node = tree->root;
    if (node) {
        krbnode_destroy_recursive(node);
    }
    destroy(tree);
}

void krbtree_insert(krbtree_t* tree, krbnode_t* z) {
    krbnode_t* y = 0;
    krbnode_t* x = 0;
    verify(tree);
    verify(z);
    if (!tree->root) { /* 没有任何节点 */
        z->color  = rb_color_black;
        z->left   = nil;
        z->right  = nil;
        z->parent = nil;
        tree->root   = z;
        return;
    }  
    x = tree->root;
    y = nil;
    while (x != nil) {
        y = x;
        if (z->key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }
    z->parent = y;
    if (y == nil) {
        tree->root = z;
        tree->root->parent = nil;
        tree->root->parent->color = rb_color_black;
    } else if (z->key < y->key) {
        y->left = z;
    } else {
        y->right = z;
    }
    /* 调整红黑树 */
    krbtree_insert_fixup(tree, z);
}

krbnode_t* krbnode_find(krbnode_t* node, uint64_t key) {
    if (!node) {
        return 0;
    }
    while (node != nil) {
        if (node->key < key) {
            node = node->right;
        } else if (node->key > key) {
            node = node->left;
        } else {
            break;
        }
    }
    return ((node != nil) ? node : 0);
}

krbnode_t* krbtree_find(krbtree_t* tree, uint64_t key) {
    verify(tree);
    return krbnode_find(tree->root, key);
}

void krbtree_delete(krbtree_t* tree, krbnode_t* z) {
    krbnode_t* x = 0;
    krbnode_t* y = 0;
    verify(tree);
    verify(z);
    if (z->left == nil || z->right == nil) {
        y = z;
    } else {
        y = krbnode_get_successor(z);
    }
    if (y->left != nil) {
        x = y->left;
    } else {
        x = y->right;
    }
    x->parent = y->parent;
    if (y->parent == nil) {
        tree->root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }
    if (y != z) {
        z->key = y->key;
    }
    if (y->color == rb_color_black) {
        krbtree_delete_fixup(tree, x);
    }
    /* 销毁 */
    krbnode_destroy(z);
}

krbnode_t* krbtree_min(krbtree_t* tree) {
    krbnode_t* x = 0;
    krbnode_t* t = 0;
    verify(tree);
    if (!tree->root) {
        return 0;
    }
    x = tree->root;
    while (x != nil) {
        t = x;
        x = x->left;
    }
    return t;
}

krbnode_t* krbtree_max(krbtree_t* tree) {
    krbnode_t* x = 0;
    krbnode_t* t = 0;
    verify(tree);
    if (!tree->root) {
        return 0;
    }
    x = tree->root;
    while (x != nil) {
        t = x;
        x = x->right;
    }
    return t;
}

void krbtree_left_rotate(krbtree_t* tree, krbnode_t* x) {
    krbnode_t* y = 0;
    verify(tree);
    verify(x);
    if (x->right == nil) {
        return;
    }
    y = x->right;
    x->right = y->left;
    if (y->left != nil) {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nil) {
        tree->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left   = x;
    x->parent = y;
}

void krbtree_right_rotate(krbtree_t* tree, krbnode_t* x) {
    krbnode_t* y = 0;
    verify(tree);
    verify(x);
    if (x->left == nil) {
        return;
    }
    y = x->left;
    x->left = y->right;
    if (y->right != nil) {
        y->right->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nil) {
        tree->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->right  = x;
    x->parent = y;
}

void krbtree_insert_fixup(krbtree_t* tree, krbnode_t* z) {
    krbnode_t* y = 0;
    verify(tree);
    verify(z);
    while (z->parent->color == rb_color_red) {
        if (z->parent == z->parent->parent->left) {
            y = z->parent->parent->right;
            if (y->color == rb_color_red) {
                y->color                 = rb_color_black;
                z->parent->color         = rb_color_black;
                z->parent->parent->color = rb_color_red;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    krbtree_left_rotate(tree, z);
                }
                z->parent->color         = rb_color_black;
                z->parent->parent->color = rb_color_red;
                krbtree_right_rotate(tree, z->parent->parent);
            }
        } else {
            y = z->parent->parent->left;
            if (y->color == rb_color_red) {
                y->color                 = rb_color_black;
                z->parent->color         = rb_color_black;
                z->parent->parent->color = rb_color_red;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    krbtree_right_rotate(tree, z);
                }
                z->parent->color         = rb_color_black;
                z->parent->parent->color = rb_color_red;
                krbtree_left_rotate(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = rb_color_black;
}

void krbtree_delete_fixup(krbtree_t* tree, krbnode_t* x) {
    krbnode_t* w = 0;
    verify(tree);
    verify(x);
    while ((x != tree->root) && (x->color == rb_color_black)) {
        if (x == x->parent->left) {
            w = x->parent->right;
            if (w->color == rb_color_red) {
                w->color         = rb_color_black;
                x->parent->color = rb_color_red;
                krbtree_left_rotate(tree, x->parent);
                w = x->parent->right;
            }
            if (w == nil) {
                break;
            }
            if ((w->left->color == rb_color_black) &&
                (w->right->color == rb_color_black)) {
                w->color = rb_color_red;
                x        = x->parent;
            } else {
                if (w->right->color == rb_color_black) {
                    w->color       = rb_color_red;
                    w->left->color = rb_color_black;
                    krbtree_right_rotate(tree, w);
                    w = x->parent->right;
                }
                w->color         = x->parent->color;
                x->parent->color = rb_color_black;
                w->right->color  = rb_color_black;
                krbtree_left_rotate(tree, x->parent);
                x = tree->root;
            }
        } else {
            w = x->parent->left;
            if (w->color == rb_color_red) {
                w->color         = rb_color_black;
                x->parent->color = rb_color_red;
                krbtree_right_rotate(tree, x->parent);
                w = x->parent->left;
            }
            if (w == nil) {
                break;
            }
            if ((w->left->color == rb_color_black) &&
                (w->right->color == rb_color_black)) {
                w->color = rb_color_red;
                x = x->parent;
            } else {
                if (w->left->color == rb_color_black) {
                    w->color        = rb_color_red;
                    w->right->color = rb_color_black;
                    krbtree_left_rotate(tree, w);
                    w = x->parent->left;
                }
                w->color         = x->parent->color;
                x->parent->color = rb_color_black;
                w->left->color   = rb_color_black;
                krbtree_right_rotate(tree, x->parent);
                x = tree->root;
            }
        }
    }
    x->color = rb_color_black;
}

krbnode_t* krbnode_get_successor(krbnode_t* x) {
    krbnode_t* z = x;
    krbnode_t* y = 0;
    if (z->right != nil) {
        z = z->right;
        while (z->left != nil) {
            z = z->left;
        }
        return z;
    }
    y = z->parent;
    while ((y != nil) && (z == y->right)) {
        z = y;
        y = y->parent;
    }
    return y;
}
