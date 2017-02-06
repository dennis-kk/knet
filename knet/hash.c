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

#include "hash.h"
#include "list.h"
#include "logger.h"
#include "misc.h"

/**
 * ��ϣ��
 */
struct _hash_t {
    uint32_t          size;         /* Ͱ���� */
    uint32_t          count;        /* ��ǰ����Ԫ�ظ��� */
    kdlist_t**        buckets;      /* Ͱ���� */
    knet_hash_dtor_t  dtor;         /* �Զ���ֵ���ٺ��� */
    uint32_t          it_index;     /* ������ - ��ǰ������Ͱ���� */
    kdlist_node_t*    it_node_safe; /* ������ - ��ǰͰ�ڽڵ� */
    kdlist_node_t*    it_node_next; /* ������ - ��ǰͰ�ڽڵ����һ���ڵ� */
};

/**
 * ��ϣ���-ֵ��
 */
struct _hash_value_t {
    uint32_t key;        /* ���ּ� */
    char*    string_key; /* �ַ����� */
    void*    value;      /* ֵ */
};

/**
 * ����ֵ
 * @param key ���ּ�
 * @param string_key �ַ�����
 * @param value ֵ
 * @return khash_value_tʵ��
 */
khash_value_t* hash_value_create(uint32_t key, const char* string_key, void* value);

/**
 * ����ֵ
 * @param hash_value khash_value_tʵ��
 */
void hash_value_destroy(khash_value_t* hash_value);

/**
 * �������ּ��Ƿ����
 * @param hash_value khash_value_tʵ��
 * @param key ������
 * @retval 0 �����
 * @retval ���� ���
 */
int hash_value_equal(khash_value_t* hash_value, uint32_t key);

/**
 * �����ַ������Ƿ����
 * @param hash_value khash_value_tʵ��
 * @param key �ַ�����
 * @retval 0 �����
 * @retval ���� ���
 */
int hash_value_equal_string_key(khash_value_t* hash_value, const char* key);

/**
 * �����ַ���������ֵ
 * @param key �ַ���
 * @return ������ֵ
 */
uint32_t hash_string(const char* key);

khash_value_t* hash_value_create(uint32_t key, const char* string_key, void* value) {
    khash_value_t* hash_value = create(khash_value_t);
    verify(hash_value);
    memset(hash_value, 0, sizeof(khash_value_t));    
    if (string_key) { /* �ַ����� */
        hash_value->string_key = create_type(char, strlen(string_key) + 1);
        if (!hash_value->string_key) {
            knet_free(hash_value);
            return 0;
        }
        strcpy(hash_value->string_key, string_key);
    } else { /* ���ּ� */
        hash_value->key = key;
    }
    hash_value->value = value;
    return hash_value;
}

void hash_value_destroy(khash_value_t* hash_value) {
    verify(hash_value);
    if (hash_value->string_key) {
        knet_free(hash_value->string_key);
    }
    knet_free(hash_value);
}

void* hash_value_get_value(khash_value_t* hash_value) {
    verify(hash_value);
    return hash_value->value;
}

uint32_t hash_value_get_key(khash_value_t* hash_value) {
    verify(hash_value);
    return hash_value->key;
}

const char* hash_value_get_string_key(khash_value_t* hash_value) {
    verify(hash_value);
    return hash_value->string_key;
}

int hash_value_equal(khash_value_t* hash_value, uint32_t key) {
    verify(hash_value);
    return (key == hash_value->key);
}

int hash_value_equal_string_key(khash_value_t* hash_value, const char* key) {
    verify(hash_value);
    verify(key);
    if (!hash_value->string_key) {
        /* �ⲿ����ĵ��� */
        verify(0);
        return 0;
    }
    return (0 == strcmp(key, hash_value->string_key));
}

khash_t* hash_create(uint32_t size, knet_hash_dtor_t dtor) {
    uint32_t i    = 0;
    khash_t* hash = create(khash_t);
    verify(hash);
    memset(hash, 0, sizeof(khash_t));
    if (!size) {
        size = 64; /* Ĭ��bucket���� */
    }
    hash->dtor = dtor;
    hash->size = size;
    /* ����Ͱ���� */
    hash->buckets = (kdlist_t**)create_type(kdlist_t*, sizeof(kdlist_t*) * size);
    verify(hash->buckets);
    if (!hash->buckets) {
        knet_free(hash);
        return 0;
    }
    memset(hash->buckets, 0, size * sizeof(kdlist_t*));
    /* ����bucket���� */
    for (; i < size; i++) {
        hash->buckets[i] = dlist_create();
    }
    return hash;
}

void hash_destroy(khash_t* hash) {
    uint32_t       i     = 0;
    kdlist_node_t* node  = 0;
    kdlist_node_t* temp  = 0;
    khash_value_t* value = 0;
    verify(hash);
    /* ��������bucket������ */
    for (; i < hash->size; i++) {
        if (hash->buckets[i]) {
            dlist_for_each_safe(hash->buckets[i], node, temp) {
                value = (khash_value_t*)dlist_node_get_data(node);
                if (hash->dtor) {
                    /* �Զ������� */
                    hash->dtor(value->value);
                }
                /* ����ֵ */
                hash_value_destroy(value);
            }
            /* �������� */
            dlist_destroy(hash->buckets[i]);
            hash->buckets[i] = 0;
        }
    }
    /* ����bucket */
    knet_free(hash->buckets);
    knet_free(hash);
}

int hash_add(khash_t* hash, uint32_t key, void* value) {
    uint32_t       hash_key   = 0;
    khash_value_t* hash_value = 0;
    verify(hash);
    verify(value);
    hash_key = key % hash->size;
    /* ����ֵ */
    hash_value = hash_value_create(key, 0, value);
    verify(hash_value);
    if (!hash_value) {
        return error_no_memory;
    }
    /* ��ӵ�����β */
    if (!dlist_add_tail_node(hash->buckets[hash_key], hash_value)) {
        return error_no_memory;
    }
    hash->count++;
    return error_ok;
}

int hash_add_string_key(khash_t* hash, const char* key, void* value) {
    uint32_t       hash_key   = 0;
    khash_value_t* hash_value = 0;
    verify(hash);
    verify(value);
    hash_key = hash_string(key) % hash->size;
    /* ����ֵ */
    hash_value = hash_value_create(0, key, value);
    if (!hash_value) {
        return error_no_memory;
    }
    /* ��ӵ�����β */
    if (!dlist_add_tail_node(hash->buckets[hash_key], hash_value)) {
        return error_no_memory;
    }
    hash->count++;
    return error_ok;
}

void* hash_remove(khash_t* hash, uint32_t key) {
    uint32_t       hash_key   = 0;
    kdlist_node_t* node       = 0;
    kdlist_node_t* temp       = 0;
    khash_value_t* hash_value = 0;
    void*          value      = 0;
    verify(hash);
    hash_key = key % hash->size;
    /* ����������� */
    dlist_for_each_safe(hash->buckets[hash_key], node, temp) {
        if (node == hash->it_node_next) { /* ɾ�����ڱ����ڵ�ĺ����ڵ� */
            hash->it_node_next = dlist_next(hash->buckets[hash_key], hash->it_node_next);
        }
        hash_value = (khash_value_t*)dlist_node_get_data(node);
        if (hash_value_equal(hash_value, key)) {
            hash->count--;
            value = hash_value_get_value(hash_value);
            /* ���ٽڵ� */
            dlist_delete(hash->buckets[hash_key], node);
            /* ����ֵ */
            hash_value_destroy(hash_value);
            return value; /* ���ظ������� */
        }
    }
    return 0; /* û�ҵ� */
}

void* hash_remove_string_key(khash_t* hash, const char* key) {
    uint32_t       hash_key   = 0;
    kdlist_node_t* node       = 0;
    kdlist_node_t* temp       = 0;
    khash_value_t* hash_value = 0;
    void*          value      = 0;
    verify(hash);
    hash_key = hash_string(key) % hash->size;
    /* ����������� */
    dlist_for_each_safe(hash->buckets[hash_key], node, temp) {
        if (node == hash->it_node_next) { /* ɾ�����ڱ����ڵ�ĺ����ڵ� */
            hash->it_node_next = dlist_next(hash->buckets[hash_key], hash->it_node_next);
        }
        hash_value = (khash_value_t*)dlist_node_get_data(node);
        if (hash_value_equal_string_key(hash_value, key)) {
            hash->count--;
            value = hash_value_get_value(hash_value);
            /* ���ٽڵ� */
            dlist_delete(hash->buckets[hash_key], node);
            /* ����ֵ */
            hash_value_destroy(hash_value);
            return value; /* ���ظ������� */
        }
    }
    return 0; /* û�ҵ� */
}

int hash_delete(khash_t* hash, uint32_t key) {
    void* value = 0;
    verify(hash);
    value = hash_remove(hash, key);
    if (!value) {
        return error_hash_not_found;
    }
    if (hash->dtor) {
        hash->dtor(value);
    } else {
        /* ����ⲿ������ڴ棬�������ֲ��ṩ���ٺ������ڴ�й¶ */
    }
    return error_ok;
}

int hash_replace(khash_t* hash, uint32_t key, void* value) {
    uint32_t       hash_key   = 0;
    kdlist_node_t* node       = 0;
    khash_value_t* hash_value = 0;
    verify(hash);
    verify(value);
    hash_key = key % hash->size;
    /* ����������� */
    dlist_for_each(hash->buckets[hash_key], node) {
        hash_value = (khash_value_t*)dlist_node_get_data(node);
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

int hash_replace_string_key(khash_t* hash, const char* key, void* value) {
    uint32_t       hash_key   = 0;
    kdlist_node_t* node       = 0;
    khash_value_t* hash_value = 0;
    verify(hash);
    verify(key);
    verify(value);
    hash_key = hash_string(key) % hash->size;
    /* ����������� */
    dlist_for_each(hash->buckets[hash_key], node) {
        hash_value = (khash_value_t*)dlist_node_get_data(node);
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

int hash_delete_string_key(khash_t* hash, const char* key) {
    void* value = 0;
    verify(hash);
    value = hash_remove_string_key(hash, key);
    if (!value) {
        return error_hash_not_found;
    }
    if (hash->dtor) {
        hash->dtor(value);
    } else {
        /* ����ⲿ������ڴ棬�������ֲ��ṩ���ٺ�����������ڴ�й¶ */
    }
    return error_ok;
}

void* hash_get(khash_t* hash, uint32_t key) {
    uint32_t       hash_key   = 0;
    kdlist_node_t* node       = 0;
    khash_value_t* hash_value = 0;
    verify(hash);
    hash_key = key % hash->size;
    /* ����������� */
    dlist_for_each(hash->buckets[hash_key], node) {
        hash_value = (khash_value_t*)dlist_node_get_data(node);
        if (hash_value_equal(hash_value, key)) {
            return hash_value_get_value(hash_value);
        }
    }
    return 0; /* û�ҵ� */
}

void* hash_get_string_key(khash_t* hash, const char* key) {
    uint32_t       hash_key   = 0;
    kdlist_node_t* node       = 0;
    khash_value_t* hash_value = 0;
    verify(hash);
    verify(key);
    hash_key = hash_string(key) % hash->size;
    /* ����������� */
    dlist_for_each(hash->buckets[hash_key], node) {
        hash_value = (khash_value_t*)dlist_node_get_data(node);
        if (hash_value_equal_string_key(hash_value, key)) {
            return hash_value_get_value(hash_value);
        }
    }
    return 0; /* û�ҵ� */
}

uint32_t hash_get_size(khash_t* hash) {
    verify(hash);
    return hash->count;
}

uint32_t hash_string(const char* key) {
    uint32_t hash_key = 0;
    /* �򵥵ļ��������key */
    for(; *key; key++) {
        hash_key = *key + hash_key * 31;
    }
    return hash_key;
}

khash_value_t* hash_get_first(khash_t* hash) {
    uint32_t       i          = 0;
    kdlist_t*      list       = 0;
    khash_value_t* hash_value = 0;
    verify(hash);
    hash->it_index     = 0;
    hash->it_node_safe = 0;
    hash->it_node_next = 0;
    if (!hash->count) { /* û��Ԫ�� */
        return 0;
    }
    /* �ҵ���һ����Ԫ�ص�Ͱ */
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
    /* Ԥ��ȡ��һ��Ԫ�أ������Ƿ���Ԫ�� */
    hash->it_node_next = dlist_next(list, hash->it_node_safe);
    hash_value = (khash_value_t*)dlist_node_get_data(hash->it_node_safe);
    verify(hash_value);
    return hash_value;
}

khash_value_t* hash_next(khash_t* hash) {
    uint32_t       i          = 0;
    kdlist_t*      list       = 0;
    khash_value_t* hash_value = 0;
    verify(hash);
    if (!hash->it_node_safe) { /* δ����hash_get_first() */
        hash_get_first(hash);
    }
    if (hash->it_node_next) { /* ��ǰͰ������һ��Ԫ�� */
        hash->it_node_safe = hash->it_node_next; /* ���� */
    } else { /* ����һ����Ԫ�ص�Ͱ */
        hash->it_node_safe = 0;
        if (hash->it_index >= hash->size) {
            return 0;
        }
        /* δ����β�� */
        hash->it_index++;
        /* ��������Ͱ */
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
        if (!hash->it_node_safe) { /* ����Ͱ��Ϊ�� */
            return 0;
        }
    }
    /* Ԥ��ȡ��һ��Ԫ�أ������Ƿ���Ԫ�� */
    hash->it_node_next = dlist_next(list, hash->it_node_safe);
    hash_value = (khash_value_t*)dlist_node_get_data(hash->it_node_safe);
    verify(hash_value);
    return hash_value;
}
