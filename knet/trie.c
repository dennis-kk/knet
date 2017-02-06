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

#include "trie_api.h"
#include "logger.h"

typedef struct _trie_node_t {
    struct _trie_node_t* parent;   /* ���ڵ� */
    struct _trie_node_t* left;     /* ������ */
    struct _trie_node_t* center;   /* ������ */
    struct _trie_node_t* right;    /* ������ */
    uint32_t             ref;      /* �ڵ����ü���, ����Ϊ��ſ������ٽڵ� */
    char                 key;      /* �� */
    char*                real_key; /* ĩ�˽ڵ� */
    void*                value;    /* ֵ */
} ktrie_node_t;

struct _trie_t {
    ktrie_node_t* root; /* ���ڵ� */
};

/**
 * ����trie�ڵ�
 * @param parent ktrie_node_t���ڵ�
 * @return ktrie_node_tʵ��
 */
ktrie_node_t* _trie_node_create(ktrie_node_t* parent);

/**
 * ����trie�ڵ㣬�ݹ������ӽڵ�
 * @param node ktrie_node_tʵ��
 * @param dtor �ڵ�ֵ���ٺ���
 */
void _trie_node_destroy(ktrie_node_t* node, knet_trie_dtor_t dtor);

/**
 * ����trie�ڵ�
 * @param node ktrie_node_tʵ��
 * @param dtor �ڵ�ֵ���ٺ���
 */
void _trie_node_destroy_self(ktrie_node_t* node, knet_trie_dtor_t dtor);

/**
 * ����trie�ڵ�
 * @param node ktrie_node_tʵ��
 */
void _trie_node_reset(ktrie_node_t* node);

/**
 * ����real_key�ֶ�
 * @param node ktrie_node_tʵ��
 * @param real_key ��
 */
void _trie_node_set_real_key(ktrie_node_t* node, const char* real_key);

/**
 * ����trie�ڵ�
 * @param node ktrie_node_tʵ��
 * @param key ��
 * @param s �Ӽ�
 * @param value ֵ
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int _trie_node_insert(ktrie_node_t* node, const char* key, const char* s, void* value);

/**
 * ����trie�ڵ�
 * @param node ktrie_node_tʵ��
 * @param key ��
 * @param s �Ӽ�
 * @param value ֵ
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int _trie_node_find(ktrie_node_t* node, const char* key, const char* s, void** value);

/**
 * ɾ��trie�ڵ�
 * @param node ktrie_node_tʵ��
 * @param s �Ӽ�
 * @param value ֵ
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int _trie_node_remove(ktrie_node_t* node, const char* s, void** value);

/**
 * ����trie�ڵ㣬����һ���µ�·���ڵ�
 * @param node ktrie_node_tʵ��
 * @param key ��
 * @param s �Ӽ�
 * @param value ֵ
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int _trie_node_insert_new_path(ktrie_node_t* node, const char* key, const char* s, void* value);

/**
 * ����trie�ڵ㣬ʹ�����е�·���ڵ�
 * @param node ktrie_node_tʵ��
 * @param key ��
 * @param s �Ӽ�
 * @param value ֵ
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int _trie_node_insert_exist_path(ktrie_node_t* node, const char* key, const char* s, void* value);

/**
 * ����·���ڵ�����ü���
 * @param node ktrie_node_tʵ��
 * @param s �Ӽ�
 * @param start_node ���ص���ʼ�ڵ�
 * @retval 0 ��������
 * @retval ���� ��������
 */
int _trie_node_decref_path(ktrie_node_t* node, ktrie_node_t** start_node, const char* s);

/**
 * ����·���ڵ�
 * @param node ktrie_node_tʵ��
 * @param s �Ӽ�
 */
void _trie_node_delete_path(ktrie_node_t* node, const char* s);

/**
 * �����ڵ�
 * @param node ktrie_node_tʵ��
 * @param func ��������
 * @param param ������������
 */
int _trie_node_for_each(ktrie_node_t* node, knet_trie_for_each_func_t func, void* param);

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
    knet_free(trie);
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

int trie_for_each(ktrie_t* trie, knet_trie_for_each_func_t func, void* param) {
    verify(trie);
    verify(func);
    if (_trie_node_for_each(trie->root, func, param)) {
        return error_trie_for_each_fail;
    }
    return error_ok;
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
    if (node->parent) { /* �Ǹ��ڵ� */
        if (node->parent->left == node) {
            node->parent->left = 0;
        } else if (node->parent->right == node) {
            node->parent->right = 0;
        } else if (node->parent->center == node) {
            node->parent->center = 0;
        }
    }
    verify(node);
    if (node->real_key) {
        knet_free(node->real_key);
    }
    if (dtor) {
        if (node->value) {
            dtor(node->value);
        }
    }
    knet_free(node);
}

void _trie_node_reset(ktrie_node_t* node) {
    verify(node);
    if (node->real_key) {
        knet_free(node->real_key);
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
    if (node->key == c) { /* �м�ڵ� */
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
    } else if (node->key > c) { /* ��߽ڵ� */
        if (node->left) {
            return _trie_node_find(node->left, key, s, value);
        }
    } else if (node->key < c) { /* �ұ߽ڵ� */
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
    if (n) { /* ����δ������ַ� */
        /* �����½ڵ�Ĭ��Ϊ�м�ڵ� */
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
    } else { /* ȫ��������� */        
        node->value = value;
        node->ref  += 1;
        if (node->real_key) { /* key�Ѿ����� */
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
    if (n) { /* ����δ������ַ� */
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
    } else { /* ȫ��������� */
        if (key == s) {
            if (node->key == c) {
                return error_trie_key_exist;
            } else if (node->key < c) {
                if (!node->right) {
                    node->right = _trie_node_create(node);
                    verify(node->right);
                    node->right->key   = c;
                    node->right->value = value;
                    node->right->ref  += 1;
                    if (node->right->real_key) { /* key�Ѿ����� */
                        return error_trie_key_exist;
                    }
                    _trie_node_set_real_key(node->right, key);
                } else {
                    error = _trie_node_insert(node->right, key, s, value);
                }
            } else if (node->key > c) {
                if (!node->left) {
                    node->left = _trie_node_create(node);
                    verify(node->left);
                    node->left->key   = c;
                    node->left->value = value;
                    node->left->ref  += 1;
                    if (node->left->real_key) { /* key�Ѿ����� */
                        return error_trie_key_exist;
                    }
                    _trie_node_set_real_key(node->left, key);
                } else {
                    error = _trie_node_insert(node->left, key, s, value);
                }
            }
        } else {
            node->key   = c;
            node->value = value;
            node->ref  += 1;
            if (node->real_key) { /* key�Ѿ����� */
                return error_trie_key_exist;
            }
            _trie_node_set_real_key(node, key);
        }
    }
    return error;
}

int _trie_node_insert(ktrie_node_t* node, const char* key, const char* s, void* value) {
    int  error = error_ok;
    verify(node);
    verify(s);
    if (!node->key) { /* �½����Ľڵ� */
        error = _trie_node_insert_new_path(node, key, s, value);
    } else {
        /* �ڵ��Ѿ����� */
        error = _trie_node_insert_exist_path(node, key, s, value);
    }
    return error;
}

int _trie_node_check_orphan(ktrie_node_t* node) {
    return ((!node->center && !node->left && !node->right) && (node->parent) && (!node->ref));
}

int _trie_node_remove(ktrie_node_t* node, const char* s, void** value) {
    int           error       = error_ok;
    ktrie_node_t* start_node  = 0;
    ktrie_node_t* parent_node = 0;
    verify(node);
    verify(s);
    error = _trie_node_find(node, s, s, value);
    if (error_ok != error) {
        return error;
    }
    /* ����·�����ü��� */
    if (_trie_node_decref_path(node, &start_node, s)) {
        /* ����·�� */
        _trie_node_delete_path(start_node, s);
        if (_trie_node_check_orphan(start_node)) {
            _trie_node_destroy_self(start_node, 0);
        }
        start_node = 0;
    }
    if (start_node) {
        /* ʹ��ǰ�ڵ���Ч */
        knet_free(start_node->real_key);
        start_node->real_key = 0;
        while (start_node && _trie_node_check_orphan(start_node)) {
            parent_node = start_node->parent;
            _trie_node_destroy_self(start_node, 0);
            start_node = parent_node;
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
        s += 1;
        n = *s;
        node->ref -= 1;
        if (!*start_node || (!node->ref)) {
            *start_node = node;
        }
        if (!n) {
            return (!node->ref && !node->center && !node->left && !node->right);
        }
        if (node->center) {
            ret &= _trie_node_decref_path(node->center, start_node, s);
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
    return (ret && *start_node);
}

void _trie_node_delete_path(ktrie_node_t* node, const char* s) {
    char c = 0;
    verify(node);
    verify(s);
    c = *s;
    s += 1;
    if (node->key == c) {
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

int _trie_node_for_each(ktrie_node_t* node, knet_trie_for_each_func_t func, void* param) {
    verify(node);
    verify(func);
    if (node->left) {
        if (_trie_node_for_each(node->left, func, param)) {
            return 1;
        }
    }
    if (node->center) {
        if (_trie_node_for_each(node->center, func, param)) {
            return 1;
        }
    }
    if (node->right) {
        if (_trie_node_for_each(node->right, func, param)) {
            return 1;
        }
    }
    return func(node->real_key, param);
}
