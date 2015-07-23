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

#include "trie_api.h"

typedef struct _trie_node_t {
    struct _trie_node_t* parent;   /* 父节点 */
    struct _trie_node_t* left;     /* 左子树 */
    struct _trie_node_t* center;   /* 中子树 */
    struct _trie_node_t* right;    /* 右子树 */
    uint32_t             ref;      /* 节点引用计数 */
    char                 key;      /* 键 */
    char*                real_key; /* 末端节点 */
    void*                value;    /* 值 */
} ktrie_node_t;

struct _trie_t {
    ktrie_node_t* root; /* 根节点 */
};

/**
 * 建立trie节点
 * @param parent ktrie_node_t父节点
 * @return ktrie_node_t实例
 */
ktrie_node_t* _trie_node_create(ktrie_node_t* parent);

/**
 * 销毁trie节点，递归销毁子节点
 * @param node ktrie_node_t实例
 * @param dtor 节点值销毁函数
 */
void _trie_node_destroy(ktrie_node_t* node, knet_trie_dtor_t dtor);

/**
 * 销毁trie节点
 * @param node ktrie_node_t实例
 * @param dtor 节点值销毁函数
 */
void _trie_node_destroy_self(ktrie_node_t* node, knet_trie_dtor_t dtor);

/**
 * 重置trie节点
 * @param node ktrie_node_t实例
 */
void _trie_node_reset(ktrie_node_t* node);

/**
 * 设置real_key字段
 * @param node ktrie_node_t实例
 * @param real_key 键
 */
void _trie_node_set_real_key(ktrie_node_t* node, const char* real_key);

/**
 * 建立trie节点
 * @param node ktrie_node_t实例
 * @param key 键
 * @param s 子键
 * @param value 值
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _trie_node_insert(ktrie_node_t* node, const char* key, const char* s, void* value);

/**
 * 查找trie节点
 * @param node ktrie_node_t实例
 * @param key 键
 * @param s 子键
 * @param value 值
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _trie_node_find(ktrie_node_t* node, const char* key, const char* s, void** value);

/**
 * 删除trie节点
 * @param node ktrie_node_t实例
 * @param s 子键
 * @param value 值
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _trie_node_remove(ktrie_node_t* node, const char* s, void** value);

/**
 * 插入trie节点，建立一个新的路径节点
 * @param node ktrie_node_t实例
 * @param key 键
 * @param s 子键
 * @param value 值
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _trie_node_insert_new_path(ktrie_node_t* node, const char* key, const char* s, void* value);

/**
 * 插入trie节点，使用已有的路径节点
 * @param node ktrie_node_t实例
 * @param key 键
 * @param s 子键
 * @param value 值
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _trie_node_insert_exist_path(ktrie_node_t* node, const char* key, const char* s, void* value);

/**
 * 减少路径节点的引用计数
 * @param node ktrie_node_t实例
 * @param s 子键
 * @param start_node 返回的起始节点
 * @retval 0 不能销毁
 * @retval 非零 可以销毁
 */
int _trie_node_decref_path(ktrie_node_t* node, ktrie_node_t** start_node, const char* s);

/**
 * 销毁路径节点
 * @param node ktrie_node_t实例
 * @param s 子键
 */
void _trie_node_delete_path(ktrie_node_t* node, const char* s);

ktrie_t* trie_create() {
    ktrie_t* trie = create(ktrie_t);
    verify(trie);
    memset(trie, 0, sizeof(ktrie_t));
    trie->root = _trie_node_create(0);
    verify(trie->root);
    return trie;
}

void trie_destroy(ktrie_t* trie, knet_trie_dtor_t dtor) {
    verify(trie);
    _trie_node_destroy(trie->root, dtor);
    destroy(trie);
}

int trie_insert(ktrie_t* trie, const char* s, void* value) {
    verify(trie);
    verify(s);
    verify(*s);
    return _trie_node_insert(trie->root, s, s, value);
}

int trie_find(ktrie_t* trie, const char* s, void** value) {
    verify(trie);
    verify(s);
    verify(*s);
    if (value) {
        *value = 0;
    }
    return _trie_node_find(trie->root, s, s, value);
}

int trie_remove(ktrie_t* trie, const char* s, void** value) {
    verify(trie);
    verify(s);
    verify(*s);
    if (value) {
        *value = 0;
    }
    return _trie_node_remove(trie->root, s, value);
}

ktrie_node_t* _trie_node_create(ktrie_node_t* parent) {
    ktrie_node_t* node = create(ktrie_node_t);
    verify(node);
    memset(node, 0, sizeof(ktrie_node_t));
    node->ref    = 0;
    node->parent = parent;
    return node;
}

void _trie_node_destroy(ktrie_node_t* node, knet_trie_dtor_t dtor) {
    verify(node);
    if (node->left) {
        _trie_node_destroy(node->left, dtor);
    }
    if (node->center) {
        _trie_node_destroy(node->center, dtor);
    }
    if (node->right) {
        _trie_node_destroy(node->right, dtor);
    }
    _trie_node_destroy_self(node, dtor);
}

void _trie_node_destroy_self(ktrie_node_t* node, knet_trie_dtor_t dtor) {
    verify(node);
    if (node->real_key) {
        destroy(node->real_key);
    }
    if (dtor) {
        if (node->value) {
            dtor(node->value);
        }
    }
    destroy(node);
}

void _trie_node_reset(ktrie_node_t* node) {
    verify(node);
    if (node->real_key) {
        destroy(node->real_key);
    }
    memset(node, 0, sizeof(ktrie_node_t));
}

int _trie_node_find(ktrie_node_t* node, const char* key, const char* s, void** value) {
    char c = 0;
    char n = 0;
    verify(node);
    verify(s);
    verify(key);
    if (!node->key) {
        return error_trie_not_found;
    }
    c = *s;
    if (node->key == c) {
        s += 1;
        n = *s;
        if (!n) {
            if (!node->real_key) {
                return error_trie_not_found;
            }
            if (strcmp(node->real_key, key)) {
                return error_trie_not_found;
            }
            if (value) {
                *value = node->value;
            }
            return error_ok;
        }
        if (node->center) {
            return _trie_node_find(node->center, key, s, value);
        }
    } else if (node->key > c) {
        if (node->left) {
            return _trie_node_find(node->left, key, s, value);
        }
    } else if (node->key < c) {
        if (node->right) {
            return _trie_node_find(node->right, key, s, value);
        }
    }
    return error_trie_not_found;
}

void _trie_node_set_real_key(ktrie_node_t* node, const char* real_key) {
    int key_len    = 0;
    node->real_key = 0;
    verify(node);
    verify(real_key);
    key_len        = strlen(real_key) + 1;
    node->real_key = create_raw(key_len);
    memcpy(node->real_key, real_key, key_len);
    node->real_key[key_len - 1] = 0;
}

int _trie_node_insert_new_path(ktrie_node_t* node, const char* key, const char* s, void* value) {
    char c     = 0;
    char n     = 0;
    int  error = error_ok;
    verify(node);
    verify(key);
    verify(s);
    c = s[0];
    n = s[1];
    node->key = c;
    if (n) { /* 还有未处理的字符 */
        /* 所有新节点默认为中间节点 */
        node->center = _trie_node_create(node);
        verify(node->center);
        s += 1;
        error = _trie_node_insert(node->center, key, s, value);
        if (error_ok == error) {
            node->ref += 1;
        } else {
            _trie_node_destroy(node->center, 0);
            node->center = 0;
        }
    } else { /* 全部处理完毕 */        
        node->value = value;
        node->ref  += 1;
        if (node->real_key) { /* key已经存在 */
            return error_trie_key_exist;
        }
        _trie_node_set_real_key(node, key);
    }
    return error;
}

int _trie_node_insert_exist_path(ktrie_node_t* node, const char* key, const char* s, void* value) {
    char c    = s[0];
    char n    = s[1];
    int error = error_ok;
    if (n) { /* 还有未处理的字符 */
        if (node->key == c) {
            if (!node->center) {
                node->center = _trie_node_create(node);
                verify(node->center);
            }
            s += 1;
            error = _trie_node_insert(node->center, key, s, value);
            if (error_ok == error) {
                node->ref += 1;
            }
        } else if (node->key < c) {
            if (!node->right) {
                node->right = _trie_node_create(node);
                verify(node->right);
            }
            error = _trie_node_insert(node->right, key, s, value);
        } else if (node->key > c) {
            if (!node->left) {
                node->left = _trie_node_create(node);
                verify(node->left);
            }
            error = _trie_node_insert(node->left, key, s, value);
        }
    } else { /* 全部处理完毕 */
        node->key   = c;
        node->value = value;
        node->ref  += 1;
        if (node->real_key) { /* key已经存在 */
            return error_trie_key_exist;
        }
        _trie_node_set_real_key(node, key);
    }
    return error;
}

int _trie_node_insert(ktrie_node_t* node, const char* key, const char* s, void* value) {
    int  error = error_ok;
    verify(node);
    verify(s);
    if (!node->key) { /* 新建立的节点 */
        error = _trie_node_insert_new_path(node, key, s, value);
    } else {
        /* 节点已经存在 */
        error = _trie_node_insert_exist_path(node, key, s, value);
    }
    return error;
}

int _trie_node_remove(ktrie_node_t* node, const char* s, void** value) {
    int           error      = error_ok;
    ktrie_node_t* start_node = 0;
    verify(node);
    verify(s);
    error = _trie_node_find(node, s, s, value);
    if (error_ok != error) {
        return error;
    }
    /* 减少路径引用计数 */
    if (_trie_node_decref_path(node, &start_node, s)) {
        /* 销毁路径 */
        _trie_node_delete_path(start_node, s);
        if (start_node->parent) { /* 非根节点 */
            if (start_node->parent->left == start_node) {
                start_node->parent->left = 0;
                _trie_node_destroy_self(start_node, 0);
            } else if (start_node->parent->right == start_node) {
                start_node->parent->right = 0;
                _trie_node_destroy_self(start_node, 0);
            } else if (start_node->parent->center == start_node) {
                start_node->parent->center = 0;
                _trie_node_destroy_self(start_node, 0);
            }
        } else {
            /* 根节点 */
            _trie_node_reset(start_node);
        }
    }
    return error;
}

int _trie_node_decref_path(ktrie_node_t* node, ktrie_node_t** start_node, const char* s) {
    char c   = 0;
    char n   = 0;
    int  ret = 1;
    verify(node);
    verify(s);
    c = *s;
    if (node->key == c) {
        if (!*start_node) {
            *start_node = node;
        }
        node->ref -= 1;
        s += 1;
        n = *s;
        if (!n) {
            return (!node->ref && !node->center && !node->left && !node->right);
        }
        if (node->center) {
            ret &= _trie_node_decref_path(node->center,start_node, s);
        }
    } else if (node->key > c) {
        if (node->left) {
            ret &= _trie_node_decref_path(node->left, start_node, s);
        }
    } else if (node->key < c) {
        if (node->right) {
            ret &= _trie_node_decref_path(node->right, start_node, s);
        }
    }
    return ret;
}

void _trie_node_delete_path(ktrie_node_t* node, const char* s) {
    char c = 0;
    char n = 0;
    verify(node);
    verify(s);
    c = *s;
    if (node->key == c) {
        s += 1;
        n = *s;
        if (!n) {
            return;
        }
        if (node->center) {
            _trie_node_delete_path(node->center, s);
            _trie_node_destroy_self(node->center, 0);
            node->center = 0;
        }
    } else if (node->key > c) {
        if (node->left) {
            _trie_node_delete_path(node->left, s);
            _trie_node_destroy_self(node->left, 0);
            node->left = 0;
        }
    } else if (node->key < c) {
        if (node->right) {
            _trie_node_delete_path(node->right, s);
            _trie_node_destroy_self(node->right, 0);
            node->right = 0;
        }
    }
}
