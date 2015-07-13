#ifndef FRAMEWORK_WORKER_H
#define FRAMEWORK_WORKER_H

#include "config.h"

/**
 * 建立框架工作线程
 * @param f framework_t实例
 * @return framework_worker_t实例
 */
framework_worker_t* framework_worker_create(framework_t* f);

/**
 * 销毁框架工作线程
 * @param w framework_worker_t实例
 */
void framework_worker_destroy(framework_worker_t* w);

/**
 * 启动框架工作线程
 * @param w framework_worker_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int framework_worker_start(framework_worker_t* w);

/**
 * 关闭框架工作线程
 * @param w framework_worker_t实例
 */
void framework_worker_stop(framework_worker_t* w);

#endif /* FRAMEWORK_WORKER_H */
