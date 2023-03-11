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

#include "config.h"
#include "rb_tree.h"
#include "logger.h"

/*
 * ʵ�ֻ����㷨���۹��ں����������½�
 */

struct _rb_node_t {
    struct _rb_node_t*        parent; /* ���ڵ� */
    struct _rb_node_t*        left;   /* ���� */
    struct _rb_node_t*        right;  /* �Һ��� */
    uint64_t                  key;    /* �ڵ�� */
    void*                     ptr;    /* �ڵ��û�����ָ�� */
    rb_color_e                color;  /* �ڵ���ɫ */
    knet_rb_node_destroy_cb_t cb;     /* ���ٺ���ָ�� */
};

struct _rb_tree_t {
    krbnode_t* root; /* ���ڵ� */
};

/*
 * ʵ�ֻ����㷨���۹��ں����������½�
 */

/* ��Χ�ڵ� */
static krbnode_t nil_node = { 0, 0, 0, 0, 0, rb_color_black };
static krbnode_t* nil = &nil_node;

/**
 * ���ٽڵ㼰�������ӽڵ�
 * @param node �ڵ�
 */
void krbnode_destroy_recursive(krbnode_t* node);

/**
 * ��x�ڵ�����ת
 * @param tree �����
 * @param x �ڵ�
 */
void krbtree_left_rotate(krbtree_t* tree, krbnode_t* x);

/**
 * ��x�ڵ�����ת
 * @param tree �����
 * @param x �ڵ�
 */
void krbtree_right_rotate(krbtree_t* tree, krbnode_t* x);

/**
 * ���ݶ����������Ĺ������ڵ��������,ʹ����Ϻ�����Ĺ���
 * @param tree �����
 * @param x �²���Ľڵ�
 */
void krbtree_insert_fixup(krbtree_t* tree, krbnode_t* x);

/**
 * ���ݶ����������Ĺ���ɾ���ڵ��������,ʹ����Ϻ�����Ĺ���
 * @param tree �����
 * @param x ��Ҫɾ���Ľڵ�
 */
void krbtree_delete_fixup(krbtree_t* tree, krbnode_t* x);

/**
 * ����ĳ�������ڽڵ�
 * @param tree �������ڵ�
 * @param key ��
 * @return �ڵ�
 */
krbnode_t* krbnode_find(krbnode_t* tree, uint64_t key);

/**
 * ��ȡx�ĺ�̽ڵ�
 * @param x �ڵ�
 * @return �ڵ�
 */
krbnode_t* krbnode_get_successor(krbnode_t* x);

krbnode_t* krbnode_create(uint64_t key, void* ptr, knet_rb_node_destroy_cb_t cb) {
    krbnode_t* node = knet_create(krbnode_t);
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
    knet_free(node);
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
    krbtree_t* tree = knet_create(krbtree_t);
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
    knet_free(tree);
}

void krbtree_insert(krbtree_t* tree, krbnode_t* z) {
    krbnode_t* y = 0;
    krbnode_t* x = 0;
    verify(tree);
    verify(z);
    if (!tree->root) { /* û���κνڵ� */
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
    /* ��������� */
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
    /* ���� */
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
