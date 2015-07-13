#ifndef FRAMEWORK_ACCEPTOR_H
#define FRAMEWORK_ACCEPTOR_H

#include "config.h"

/**
 * 建立框架网络监听器
 * @param f framework_t实例
 * @return framework_acceptor_t实例
 */
framework_acceptor_t* framework_acceptor_create(framework_t* f);

/**
 * 销毁框架网络监听器
 * @param a framework_acceptor_t实例
 */
void framework_acceptor_destroy(framework_acceptor_t* a);

/**
 * 启动框架网络监听器
 * @param a framework_acceptor_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int framework_acceptor_start(framework_acceptor_t* a);

/**
 * 关闭框架网络监听器
 * @param a framework_acceptor_t实例
 */
void framework_acceptor_stop(framework_acceptor_t* a);

/**
 * 等待框架网络监听器关闭
 * @param a framework_acceptor_t实例
 */
void framework_acceptor_wait_for_stop(framework_acceptor_t* a);

#endif /* FRAMEWORK_ACCEPTOR_H */
