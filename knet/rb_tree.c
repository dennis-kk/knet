#include "rb_tree.h"

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

/* 外围节点 */
static krbnode_t nil_node = { 0, 0, 0, 0, 0, rb_color_black };
static krbnode_t* nil = &nil_node;

/**
 * 销毁节点及其所有子节点
 * @param node 节点
 */
static void krbnode_destroy_recursive(krbnode_t* node);

/**
 * 对x节点左旋转
 * @param tree 红黑树
 * @param x 节点
 */
static void krbtree_left_rotate(krbtree_t* tree, krbnode_t* x);

/**
 * 对x节点右旋转
 * @param tree 红黑树
 * @param x 节点
 */
static void krbtree_right_rotate(krbtree_t* tree, krbnode_t* x);

/**
 * 根据二叉搜索树的规则插入节点后修正树,使其符合红黑树的规则
 * @param tree 红黑树
 * @param x 新插入的节点
 */
static void krbtree_insert_fixup(krbtree_t* tree, krbnode_t* x);

/**
 * 根据二叉搜索树的规则删除节点后修正树,使其符合红黑树的规则
 * @param tree 红黑树
 * @param x 需要删除的节点
 */
static void krbtree_delete_fixup(krbtree_t* tree, krbnode_t* x);

/**
 * 查找某个子树内节点
 * @param tree 子树根节点
 * @param key 键
 * @return 节点
 */
static krbnode_t* krbnode_find(krbnode_t* tree, uint64_t key);

/**
 * 获取x的后继节点, 寻找右儿子的最小节点
 * @param x 节点
 * @return 节点
 */
static krbnode_t* krbnode_get_successor(krbnode_t* x);

krbnode_t* krbnode_create(uint64_t key, void* ptr, knet_rb_node_destroy_cb_t cb) {
    krbnode_t* node = create(krbnode_t);
    verify(node);
    memset(node, 0, sizeof(krbnode_t));
    node->key    = key;
    node->ptr    = ptr;
    node->cb     = cb;
    node->left   = nil;
    node->right  = nil;
    node->parent = nil;
    node->color  = rb_color_red;
    return node;
}

void krbnode_destroy(krbnode_t* node) {
    verify(node);
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
    krbnode_t* y = 0; /* 搜索树内父节点 */
    krbnode_t* x = 0;
    verify(tree);
    verify(z);
    if (!tree->root) { /* 没有任何节点 */
        z->color   = rb_color_black; /* 根节点为黑色 */
        z->left    = nil;
        z->right   = nil;
        z->parent  = nil;
        tree->root = z;
        return;
    }
    x = tree->root;
    y = nil;
    /* 首先添加到二叉搜索树, 查找需要插入的父节点 */
    while (x != nil) {
        y = x;
        if (z->key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }
    z->parent = y; /* 设置父节点 */
    if (z->key < y->key) {
        y->left = z; /* 左分支 */
    } else {
        y->right = z; /* 右分支 */
    }
    /* 调整红黑树使之符合红黑树规则 */
    krbtree_insert_fixup(tree, z);
}

krbnode_t* krbnode_find(krbnode_t* node, uint64_t key) {
    if (node == nil) {
        /* 到达外围节点 */
        return 0;
    }
    if (node->key > key) { /* 左分支 */
        return krbnode_find(node->left, key);
    } else if (node->key < key) { /* 右分支 */
        return krbnode_find(node->right, key);
    }
    return node;
}

krbnode_t* krbtree_find(krbtree_t* tree, uint64_t key) {
    verify(tree);
    /* 从根节点开始搜索 */
    return krbnode_find(tree->root, key);
}

void krbtree_delete(krbtree_t* tree, krbnode_t* z) {
    krbnode_t* x = 0;
    krbnode_t* y = 0;
    verify(tree);
    verify(z);
    if (z->left == nil || z->right == nil) { /* 至少有一个孩子 */
        y = z;
    } else {
        /* 寻找后继节点 */
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
        z->cb  = y->cb;
        z->ptr = y->ptr;
    }
    if (y->color == rb_color_black) {
        /* 调整红黑树使之符合红黑树规则 */
        krbtree_delete_fixup(tree, x);
    }
    /* 销毁 */
    krbnode_destroy(y);
}

krbnode_t* krbtree_min(krbtree_t* tree) {
    krbnode_t* x = 0;
    krbnode_t* t = 0;
    verify(tree);
    if (tree->root == nil) {
        return 0;
    }
    x = tree->root;
    /* 最左边节点为最小节点 */
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
    if (tree->root == nil) {
        return 0;
    }
    /* 最右边的节点为最大节点 */
    x = tree->root;
    while (x != nil) {
        t = x;
        x = x->right;
    }
    return x;
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
    verify(x);
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
