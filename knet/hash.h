#ifndef HASH_H
#define HASH_H

#include "config.h"

/*
 * 内部使用，哈希表，同时支持数字或字符串作为key
 */

/*
 * 建立哈希表
 * @param size 哈希表桶数量, 0将使用默认桶数量
 * @param dtor 用户自定义值销毁函数
 * @return hash_t实例
 */
hash_t* hash_create(uint32_t size, hash_dtor_t dtor);

/*
 * 销毁哈希表
 * @param hash hash_t实例
 */
void hash_destroy(hash_t* hash);

/*
 * 添加元素
 * @param hash hash_t实例
 * @param key 键
 * @param value 值
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int hash_add(hash_t* hash, uint32_t key, void* value);

/*
 * 添加元素
 * @param hash hash_t实例
 * @param key 字符串键
 * @param value 值
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int hash_add_string_key(hash_t* hash, const char* key, void* value);

/*
 * 移除元素
 * @param hash hash_t实例
 * @param key 键
 * @retval 0 未找到
 * @retval 有效指针 值
 */
void* hash_remove(hash_t* hash, uint32_t key);

/*
 * 移除元素
 * @param hash hash_t实例
 * @param key 字符串键
 * @retval 0 未找到
 * @retval 有效指针 值
 */
void* hash_remove_string_key(hash_t* hash, const char* key);

/*
 * 销毁元素
 * @param hash hash_t实例
 * @param key 键
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int hash_delete(hash_t* hash, uint32_t key);

/*
 * 销毁元素
 * @param hash hash_t实例
 * @param key 字符串键
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int hash_delete_string_key(hash_t* hash, const char* key);

/*
 * 获取元素
 * @param hash hash_t实例
 * @param key 键
 * @retval 0 未找到
 * @retval 有效指针 值
 */
void* hash_get(hash_t* hash, uint32_t key);

/*
 * 获取元素
 * @param hash hash_t实例
 * @param key 字符串键
 * @retval 0 未找到
 * @retval 有效指针 值
 */
void* hash_get_string_key(hash_t* hash, const char* key);

/*
 * 取得元素数量
 * @param hash hash_t实例
 * @return 元素数量
 */
uint32_t hash_get_size(hash_t* hash);

#endif /* HASH_H */
