#include "hash.h"
#include "list.h"

struct _hash_t {
    uint32_t    size;    /* 桶数量 */
    uint32_t    count;   /* 当前表内元素个数 */
    dlist_t**   buckets; /* 桶数组 */
    hash_dtor_t dtor;    /* 自定义值销毁函数 */
};

typedef struct _hash_value_t {
    uint32_t key;        /* 数字键 */
    char*    string_key; /* 字符串键 */
    void*    value;      /* 值 */
} hash_value_t;

/*
 * 创建值
 * @param key 数字键
 * @param string_key 字符串键
 * @param value 值
 * @return hash_value_t实例
 */
hash_value_t* hash_value_create(uint32_t key, const char* string_key, void* value);

/*
 * 销毁值
 * @param hash_value hash_value_t实例
 */
void hash_value_destroy(hash_value_t* hash_value);

/*
 * 测试数字键是否相等
 * @param hash_value hash_value_t实例
 * @param key 整数键
 * @retval 0 不相等
 * @retval 非零 相等
 */
int hash_value_equal(hash_value_t* hash_value, uint32_t key);

/*
 * 测试字符串键是否相等
 * @param hash_value hash_value_t实例
 * @param key 字符串键
 * @retval 0 不相等
 * @retval 非零 相等
 */
int hash_value_equal_string_key(hash_value_t* hash_value, const char* key);

/*
 * 求的字符串整数键值
 * @param key 字符串
 * @return 整数键值
 */
uint32_t _hash_string(const char* key);

hash_value_t* hash_value_create(uint32_t key, const char* string_key, void* value) {
    hash_value_t* hash_value = create(hash_value_t);
    assert(hash_value);
    memset(hash_value, 0, sizeof(hash_value_t));    
    if (string_key) { /* 字符串键 */
        hash_value->string_key = create_type(char, strlen(string_key) + 1);
        strcpy(hash_value->string_key, string_key);
    } else { /* 数字键 */
        hash_value->key = key;
    }
    hash_value->value = value;
    return hash_value;
}

void hash_value_destroy(hash_value_t* hash_value) {
    assert(hash_value);
    if (hash_value->string_key) {
        destroy(hash_value->string_key);
    }
    destroy(hash_value);
}

int hash_value_equal(hash_value_t* hash_value, uint32_t key) {
    assert(hash_value);
    return (key == hash_value->key);
}

int hash_value_equal_string_key(hash_value_t* hash_value, const char* key) {
    assert(hash_value);
    assert(key);
    if (!hash_value->string_key) {
        /* 外部错误的调用 */
        assert(0);
        return 0;
    }
    return (0 == strcmp(key, hash_value->string_key));
}

hash_t* hash_create(uint32_t size, hash_dtor_t dtor) {
    uint32_t i = 0;
    hash_t* hash = create(hash_t);
    assert(hash);
    memset(hash, 0, sizeof(hash_t));
    if (!size) {
        size = 128; /* 默认bucket数量 */
    }
    hash->dtor = dtor;
    hash->size = size;
    hash->buckets = (dlist_t**)create_type(dlist_t*, sizeof(dlist_t*) * size);
    assert(hash->buckets);
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
    assert(hash);
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
    assert(hash);
    assert(value);
    hash_key = key % hash->size;
    /* 创建值 */
    hash_value = hash_value_create(key, 0, value);
    /* 添加到链表尾 */
    dlist_add_tail_node(hash->buckets[hash_key], hash_value);
    hash->count++;
    return error_ok;
}

int hash_add_string_key(hash_t* hash, const char* key, void* value) {
    uint32_t      hash_key   = 0;
    hash_value_t* hash_value = 0;
    assert(hash);
    assert(value);
    hash_key = _hash_string(key);
    /* 创建值 */
    hash_value = hash_value_create(0, key, value);
    /* 添加到链表尾 */
    dlist_add_tail_node(hash->buckets[hash_key], hash_value);
    hash->count++;
    return error_ok;
}

void* hash_remove(hash_t* hash, uint32_t key) {
    uint32_t      hash_key   = 0;
    dlist_node_t* node       = 0;
    dlist_node_t* temp       = 0;
    hash_value_t* hash_value = 0;
    void*         value      = 0;
    assert(hash);
    hash_key = key % hash->size;
    /* 遍历链表查找 */
    dlist_for_each_safe(hash->buckets[hash_key], node, temp) {
        hash_value = (hash_value_t*)dlist_node_get_data(node);
        if (hash_value_equal(hash_value, key)) {
            hash->count--;
            value = hash_value->value;
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
    assert(hash);
    hash_key = _hash_string(key);
    /* 遍历链表查找 */
    dlist_for_each_safe(hash->buckets[hash_key], node, temp) {
        hash_value = (hash_value_t*)dlist_node_get_data(node);
        if (hash_value_equal_string_key(hash_value, key)) {
            hash->count--;
            value = hash_value->value;
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
    assert(hash);
    value = hash_remove(hash, key);
    if (hash->dtor) {
        hash->dtor(value);
    } else {
        /* 如果外部分配的内存，调用者又不提供销毁函数则内存泄露 */
    }
    return error_ok;
}

int hash_delete_string_key(hash_t* hash, const char* key) {
    void* value = 0;
    assert(hash);
    value = hash_remove_string_key(hash, key);
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
    assert(hash);
    hash_key = key % hash->size;
    /* 遍历链表查找 */
    dlist_for_each(hash->buckets[hash_key], node) {
        hash_value = (hash_value_t*)dlist_node_get_data(node);
        if (hash_value_equal(hash_value, key)) {
            return hash_value->value;
        }
    }
    return 0; /* 没找到 */
}

void* hash_get_string_key(hash_t* hash, const char* key) {
    uint32_t      hash_key   = 0;
    dlist_node_t* node       = 0;
    hash_value_t* hash_value = 0;
    assert(hash);
    hash_key = _hash_string(key);
    /* 遍历链表查找 */
    dlist_for_each(hash->buckets[hash_key], node) {
        hash_value = (hash_value_t*)dlist_node_get_data(node);
        if (hash_value_equal_string_key(hash_value, key)) {
            return hash_value->value;
        }
    }
    return 0; /* 没找到 */
}

uint32_t hash_get_size(hash_t* hash) {
    assert(hash);
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
