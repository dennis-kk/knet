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

#include "hash.h"
#include "list.h"


struct _hash_t {
    uint32_t      size;         /* 桶数量 */
    uint32_t      count;        /* 当前表内元素个数 */
    dlist_t**     buckets;      /* 桶数组 */
    hash_dtor_t   dtor;         /* 自定义值销毁函数 */
    uint32_t      it_index;     /* 遍历器 - 当前遍历的桶索引 */
    dlist_node_t* it_node_safe; /* 遍历器 - 当前桶内节点 */
    dlist_node_t* it_node_next; /* 遍历器 - 当前桶内节点的下一个节点 */
};

struct _hash_value_t {
    uint32_t key;        /* 数字键 */
    char*    string_key; /* 字符串键 */
    void*    value;      /* 值 */
};

/**
 * 创建值
 * @param key 数字键
 * @param string_key 字符串键
 * @param value 值
 * @return hash_value_t实例
 */
hash_value_t* hash_value_create(uint32_t key, const char* string_key, void* value);

/**
 * 销毁值
 * @param hash_value hash_value_t实例
 */
void hash_value_destroy(hash_value_t* hash_value);

/**
 * 测试数字键是否相等
 * @param hash_value hash_value_t实例
 * @param key 整数键
 * @retval 0 不相等
 * @retval 非零 相等
 */
int hash_value_equal(hash_value_t* hash_value, uint32_t key);

/**
 * 测试字符串键是否相等
 * @param hash_value hash_value_t实例
 * @param key 字符串键
 * @retval 0 不相等
 * @retval 非零 相等
 */
int hash_value_equal_string_key(hash_value_t* hash_value, const char* key);

/**
 * 计算字符串整数键值
 * @param key 字符串
 * @return 整数键值
 */
uint32_t _hash_string(const char* key);

hash_value_t* hash_value_create(uint32_t key, const char* string_key, void* value) {
    hash_value_t* hash_value = create(hash_value_t);
    verify(hash_value);
    memset(hash_value, 0, sizeof(hash_value_t));    
    if (string_key) { /* 字符串键 */
        hash_value->string_key = create_type(char, strlen(string_key) + 1);
        if (!hash_value->string_key) {
            destroy(hash_value);
            return 0;
        }
        strcpy(hash_value->string_key, string_key);
    } else { /* 数字键 */
        hash_value->key = key;
    }
    hash_value->value = value;
    return hash_value;
}

void hash_value_destroy(hash_value_t* hash_value) {
    verify(hash_value);
    if (hash_value->string_key) {
        destroy(hash_value->string_key);
    }
    destroy(hash_value);
}

void* hash_value_get_value(hash_value_t* hash_value) {
    verify(hash_value);
    return hash_value->value;
}

uint32_t hash_value_get_key(hash_value_t* hash_value) {
    verify(hash_value);
    return hash_value->key;
}

const char* hash_value_get_string_key(hash_value_t* hash_value) {
    verify(hash_value);
    return hash_value->string_key;
}

int hash_value_equal(hash_value_t* hash_value, uint32_t key) {
    verify(hash_value);
    return (key == hash_value->key);
}

int hash_value_equal_string_key(hash_value_t* hash_value, const char* key) {
    verify(hash_value);
    verify(key);
    if (!hash_value->string_key) {
        /* 外部错误的调用 */
        verify(0);
        return 0;
    }
    return (0 == strcmp(key, hash_value->string_key));
}

hash_t* hash_create(uint32_t size, hash_dtor_t dtor) {
    uint32_t i = 0;
    hash_t* hash = create(hash_t);
    verify(hash);
    memset(hash, 0, sizeof(hash_t));
    if (!size) {
        size = 64; /* 默认bucket数量 */
    }
    hash->dtor = dtor;
    hash->size = size;
    hash->buckets = (dlist_t**)create_type(dlist_t*, sizeof(dlist_t*) * size);
    verify(hash->buckets);
    if (!hash->buckets) {
        destroy(hash);
        return 0;
    }
    memset(hash->buckets, 0, size * sizeof(dlist_t*));
    /* 创建bucket链表 */
    for (; i < size; i++) {
        hash->buckets[i] = dlist_create();
    }
    return hash;
}

void hash_destroy(hash_t* hash) {
    uint32_t      i     = 0;
    dlist_node_t* node  = 0;
    dlist_node_t* temp  = 0;
    hash_value_t* value = 0;
    verify(hash);
    /* 遍历所有bucket并销毁 */
    for (; i < hash->size; i++) {
        if (hash->buckets[i]) {
            dlist_for_each_safe(hash->buckets[i], node, temp) {
                value = (hash_value_t*)dlist_node_get_data(node);
                if (hash->dtor) {
                    /* 自定义销毁 */
                    hash->dtor(value->value);
                }
                /* 销毁值 */
                hash_value_destroy(value);
            }
            /* 销毁链表 */
            dlist_destroy(hash->buckets[i]);
            hash->buckets[i] = 0;
        }
    }
    /* 销毁bucket */
    destroy(hash->buckets);
    destroy(hash);
}

int hash_add(hash_t* hash, uint32_t key, void* value) {
    uint32_t      hash_key   = 0;
    hash_value_t* hash_value = 0;
    verify(hash);
    verify(value);
    hash_key = key % hash->size;
    /* 创建值 */
    hash_value = hash_value_create(key, 0, value);
    verify(hash_value);
    if (!hash_value) {
        return error_no_memory;
    }
    /* 添加到链表尾 */
    if (!dlist_add_tail_node(hash->buckets[hash_key], hash_value)) {
        return error_no_memory;
    }
    hash->count++;
    return error_ok;
}

int hash_add_string_key(hash_t* hash, const char* key, void* value) {
    uint32_t      hash_key   = 0;
    hash_value_t* hash_value = 0;
    verify(hash);
    verify(value);
    hash_key = _hash_string(key) % hash->size;
    /* 创建值 */
    hash_value = hash_value_create(0, key, value);
    if (!hash_value) {
        return error_no_memory;
    }
    /* 添加到链表尾 */
    if (!dlist_add_tail_node(hash->buckets[hash_key], hash_value)) {
        return error_no_memory;
    }
    hash->count++;
    return error_ok;
}

void* hash_remove(hash_t* hash, uint32_t key) {
    uint32_t      hash_key   = 0;
    dlist_node_t* node       = 0;
    dlist_node_t* temp       = 0;
    hash_value_t* hash_value = 0;
    void*         value      = 0;
    verify(hash);
    hash_key = key % hash->size;
    /* 遍历链表查找 */
    dlist_for_each_safe(hash->buckets[hash_key], node, temp) {
        if (node == hash->it_node_next) { /* 删除正在遍历节点的后续节点 */
            hash->it_node_next = dlist_next(hash->buckets[hash_key], hash->it_node_next);
        }
        hash_value = (hash_value_t*)dlist_node_get_data(node);
        if (hash_value_equal(hash_value, key)) {
            hash->count--;
            value = hash_value_get_value(hash_value);
            /* 销毁节点 */
            dlist_delete(hash->buckets[hash_key], node);
            /* 销毁值 */
            hash_value_destroy(hash_value);
            return value; /* 返回给调用者 */
        }
    }
    return 0; /* 没找到 */
}

void* hash_remove_string_key(hash_t* hash, const char* key) {
    uint32_t      hash_key   = 0;
    dlist_node_t* node       = 0;
    dlist_node_t* temp       = 0;
    hash_value_t* hash_value = 0;
    void*         value      = 0;
    verify(hash);
    hash_key = _hash_string(key) % hash->size;
    /* 遍历链表查找 */
    dlist_for_each_safe(hash->buckets[hash_key], node, temp) {
        if (node == hash->it_node_next) { /* 删除正在遍历节点的后续节点 */
            hash->it_node_next = dlist_next(hash->buckets[hash_key], hash->it_node_next);
        }
        hash_value = (hash_value_t*)dlist_node_get_data(node);
        if (hash_value_equal_string_key(hash_value, key)) {
            hash->count--;
            value = hash_value_get_value(hash_value);
            /* 销毁节点 */
            dlist_delete(hash->buckets[hash_key], node);
            /* 销毁值 */
            hash_value_destroy(hash_value);
            return value; /* 返回给调用者 */
        }
    }
    return 0; /* 没找到 */
}

int hash_delete(hash_t* hash, uint32_t key) {
    void* value = 0;
    verify(hash);
    value = hash_remove(hash, key);
    if (!value) {
        return error_hash_not_found;
    }
    if (hash->dtor) {
        hash->dtor(value);
    } else {
        /* 如果外部分配的内存，调用者又不提供销毁函数则内存泄露 */
    }
    return error_ok;
}

int hash_replace(hash_t* hash, uint32_t key, void* value) {
    uint32_t      hash_key   = 0;
    dlist_node_t* node       = 0;
    hash_value_t* hash_value = 0;
    verify(hash);
    verify(value);
    hash_key = key % hash->size;
    /* 遍历链表查找 */
    dlist_for_each(hash->buckets[hash_key], node) {
        hash_value = (hash_value_t*)dlist_node_get_data(node);
        if (hash_value_equal(hash_value, key)) {
            if (hash->dtor) {
                hash->dtor(hash_value->value);
            }
            hash_value->value = value;
            return error_ok;
        }
    }
    return hash_add(hash, key, value);
}

int hash_replace_string_key(hash_t* hash, const char* key, void* value) {
    uint32_t      hash_key   = 0;
    dlist_node_t* node       = 0;
    hash_value_t* hash_value = 0;
    verify(hash);
    verify(key);
    verify(value);
    hash_key = _hash_string(key) % hash->size;
    /* 遍历链表查找 */
    dlist_for_each(hash->buckets[hash_key], node) {
        hash_value = (hash_value_t*)dlist_node_get_data(node);
        if (hash_value_equal_string_key(hash_value, key)) {
            if (hash->dtor) {
                hash->dtor(hash_value->value);
            }
            hash_value->value = value;
            return error_ok;
        }
    }
    return hash_add_string_key(hash, key, value);
}

int hash_delete_string_key(hash_t* hash, const char* key) {
    void* value = 0;
    verify(hash);
    value = hash_remove_string_key(hash, key);
    if (!value) {
        return error_hash_not_found;
    }
    if (hash->dtor) {
        hash->dtor(value);
    } else {
        /* 如果外部分配的内存，调用者又不提供销毁函数可能造成内存泄露 */
    }
    return error_ok;
}

void* hash_get(hash_t* hash, uint32_t key) {
    uint32_t      hash_key   = 0;
    dlist_node_t* node       = 0;
    hash_value_t* hash_value = 0;
    verify(hash);
    hash_key = key % hash->size;
    /* 遍历链表查找 */
    dlist_for_each(hash->buckets[hash_key], node) {
        hash_value = (hash_value_t*)dlist_node_get_data(node);
        if (hash_value_equal(hash_value, key)) {
            return hash_value_get_value(hash_value);
        }
    }
    return 0; /* 没找到 */
}

void* hash_get_string_key(hash_t* hash, const char* key) {
    uint32_t      hash_key   = 0;
    dlist_node_t* node       = 0;
    hash_value_t* hash_value = 0;
    verify(hash);
    verify(key);
    hash_key = _hash_string(key) % hash->size;
    /* 遍历链表查找 */
    dlist_for_each(hash->buckets[hash_key], node) {
        hash_value = (hash_value_t*)dlist_node_get_data(node);
        if (hash_value_equal_string_key(hash_value, key)) {
            return hash_value_get_value(hash_value);
        }
    }
    return 0; /* 没找到 */
}

uint32_t hash_get_size(hash_t* hash) {
    verify(hash);
    return hash->count;
}

uint32_t _hash_string(const char* key) {
    uint32_t hash_key = 0;
    /* 简单的计算出整数key */
    for(; *key; key++) {
        hash_key = *key + hash_key * 31;
    }
    return hash_key;
}

hash_value_t* hash_get_first(hash_t* hash) {
    uint32_t      i          = 0;
    dlist_t*      list       = 0;
    hash_value_t* hash_value = 0;
    verify(hash);
    hash->it_index     = 0;
    hash->it_node_safe = 0;
    hash->it_node_next = 0;
    if (!hash->count) { /* 没有元素 */
        return 0;
    }
    /* 找到第一个有元素的桶 */
    for (; i < hash->size; i++) {
        list = hash->buckets[i];
        verify(list);
        if (!dlist_empty(list)) {
            hash->it_index = i;
            break;
        }
    }
    verify(list);
    if (!list) {
        return 0;
    }
    hash->it_node_safe = dlist_get_front(list);
    verify(hash->it_node_safe);
    /* 预先取下一个元素，不论是否有元素 */
    hash->it_node_next = dlist_next(list, hash->it_node_safe);
    hash_value = (hash_value_t*)dlist_node_get_data(hash->it_node_safe);
    verify(hash_value);
    return hash_value;
}

hash_value_t* hash_next(hash_t* hash) {
    uint32_t      i          = 0;
    dlist_t*      list       = 0;
    hash_value_t* hash_value = 0;
    verify(hash);
    if (!hash->it_node_safe) { /* 未调用hash_get_first() */
        hash_get_first(hash);
    }
    if (hash->it_node_next) { /* 当前桶还有下一个元素 */
        hash->it_node_safe = hash->it_node_next; /* 交换 */
    } else { /* 到下一个有元素的桶 */
        hash->it_node_safe = 0;
        if (hash->it_index >= hash->size) {
            return 0;
        }
        /* 未到达尾部 */
        hash->it_index++;
        /* 遍历后续桶 */
        for (i = hash->it_index; i < hash->size; i++) {
            list = hash->buckets[i];
            verify(list);
            if (!dlist_empty(list)) {
                hash->it_node_safe = dlist_get_front(list);
                verify(hash->it_node_safe);
                hash->it_index = i;
                break;
            }
        }
        if (!hash->it_node_safe) { /* 后续桶都为空 */
            return 0;
        }
    }
    /* 预先取下一个元素，不论是否有元素 */
    hash->it_node_next = dlist_next(list, hash->it_node_safe);
    hash_value = (hash_value_t*)dlist_node_get_data(hash->it_node_safe);
    verify(hash_value);
    return hash_value;
}
