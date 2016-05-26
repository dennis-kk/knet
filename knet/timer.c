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
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "timer.h"
#include "list.h"
#include "misc.h"
#include "logger.h"
#include "rb_tree.h"

/**
 * 定时器
 */
struct _ktimer_t {
    kdlist_t*       current_list;  /* 所属链表 */
    kdlist_node_t*  list_node;     /* 链表节点 */
    ktimer_loop_t*  timer_loop;    /* 定时器循环 */
    ktimer_type_e   type;          /* 定时器类型 */
    ktimer_cb_t     cb;            /* 定时器回调 */
    void*           data;          /* 自定义数据 */
    uint64_t        ms;            /* 下次触发时间 */
    uint64_t        intval;        /* 定时器间隔 */
    int             times;         /* 触发最大次数 */
    int             current_times; /* 当前触发次数 */
    int             stop;          /* 终止标志 */
};

/**
 * 定时器循环
 */
struct _ktimer_loop_t {
    krbtree_t* timer_tree; /* 红黑树 */
    int        running;    /* 运行标志 */
    uint64_t   last_tick;  /* 上一次调用循环的时间（毫秒） */
    uint64_t   freq;       /* 循环调用间隔(毫秒) */
};

/**
 * 销毁红黑树节点
 */
void _rb_node_destroy_cb(void* ptr, uint64_t key);

/**
 * 添加定时器
 * @param timer 定时器
 * @param node 定时器链表节点
 * @param ms 到期时间戳(毫秒)
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _ktimer_add_node(ktimer_t* timer, kdlist_node_t* node, time_t ms);

/**
 * 添加定时器
 * @param timer 定时器
 * @param ms 到期时间戳(毫秒)
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _ktimer_add(ktimer_t* timer, time_t ms);

void _rb_node_destroy_cb(void* ptr, uint64_t key) {
    kdlist_node_t* node = 0;
    kdlist_node_t* temp = 0;
    kdlist_t*      list = (kdlist_t*)ptr;
    (void)key;
    /* 销毁链表内的定时器 */
    dlist_for_each_safe(list, node, temp) {
        ktimer_t* timer = (ktimer_t*)dlist_node_get_data(node);
        ktimer_destroy(timer);
    }
    /* 销毁链表 */
    dlist_destroy(list);
}

int _ktimer_add_node(ktimer_t* timer, kdlist_node_t* node, time_t ms) {
    kdlist_t*  list    = 0;
    krbnode_t* rb_node = 0;
    /* 查找节点 */
    rb_node = krbtree_find(timer->timer_loop->timer_tree, ms);
    if (!rb_node) { /* 未找到节点 */
        list    = dlist_create();
        /* 建立红黑树节点 */
        rb_node = krbnode_create(ms, list, _rb_node_destroy_cb);
        /* 插入红黑树节点 */
        krbtree_insert(timer->timer_loop->timer_tree, rb_node);
    }
    if (rb_node) {
        /* 设置定时器所属链表 */
        timer->current_list = (kdlist_t*)krbnode_get_ptr(rb_node);
        /* 添加到链表尾部 */
        dlist_add_tail(timer->current_list, node);
        /* 链表内的节点 */
        timer->list_node = node;
    } else {
        return error_fail;
    }
    return error_ok;
}

int _ktimer_add(ktimer_t* timer, time_t ms) {
    kdlist_t*  list = 0;
    krbnode_t* rb_node = 0;
    rb_node = krbtree_find(timer->timer_loop->timer_tree, ms);
    if (!rb_node) {
        list    = dlist_create();
        rb_node = krbnode_create(time_get_milliseconds_19700101() + ms,
            list, _rb_node_destroy_cb);
        /* 插入红黑树节点 */
        krbtree_insert(timer->timer_loop->timer_tree, rb_node);
    }
    if (rb_node) {
        timer->current_list = (kdlist_t*)krbnode_get_ptr(rb_node);
        timer->list_node = dlist_add_tail_node(timer->current_list, timer);
    } else {
        return error_fail;
    }
    return error_ok;
}

ktimer_loop_t* ktimer_loop_create(time_t freq) {
    ktimer_loop_t* timer_loop = create(ktimer_loop_t);
    verify(timer_loop);
    memset(timer_loop, 0, sizeof(ktimer_loop_t));
    timer_loop->last_tick  = time_get_milliseconds_19700101();
    timer_loop->timer_tree = krbtree_create();
    timer_loop->freq       = freq;
    return timer_loop;
}

void ktimer_loop_destroy(ktimer_loop_t* timer_loop) {
    verify(timer_loop);
    /* 销毁红黑树 */
    krbtree_destroy(timer_loop->timer_tree);
    destroy(timer_loop);
}

void ktimer_loop_run(ktimer_loop_t* timer_loop) {
    verify(timer_loop);
    timer_loop->running = 1;
    while (timer_loop->running) {
        thread_sleep_ms((int)timer_loop->freq);
        ktimer_loop_run_once(timer_loop);
    }
}

void ktimer_loop_exit(ktimer_loop_t* timer_loop) {
    verify(timer_loop);
    timer_loop->running = 0;
}

int ktimer_loop_run_once(ktimer_loop_t* timer_loop) {
    krbnode_t*     rb_node = 0;
    kdlist_node_t* node    = 0;
    kdlist_node_t* temp    = 0;
    kdlist_t*      timers  = 0;
    ktimer_t*      timer   = 0;
    uint64_t       key     = 0;
    uint64_t       ms      = time_get_milliseconds_19700101(); /* 当前时间戳（毫秒） */
    int            count   = 0;
    verify(timer_loop);
    /* 记录上次tick时间戳 */
    timer_loop->last_tick = ms;
    /* 查找时间戳最小节点 */
    rb_node = krbtree_min(timer_loop->timer_tree);
    if (!rb_node) {
        /* 没有节点 */
        return 0;
    }
    /* 获取最小节点的时间戳 */
    key = krbnode_get_key(rb_node);
    while (rb_node && (key < ms)) { /* 到期 */
        /* 获取当前时间戳的定时器链表 */
        timers = (kdlist_t*)krbnode_get_ptr(rb_node);
        verify(timers);
        /* 处理所有到期的定时器 */
        dlist_for_each_safe(timers, node, temp) {
            timer = (ktimer_t*)dlist_node_get_data(node);
            verify(timer);
            if ((!timer->stop) && timer->cb) {
                /* 定时器到期,调用回调 */
                timer->cb(timer, timer->data);
                count += 1;
            }
            /* 调用次数递增 */
            timer->current_times += 1;
            if (!(timer->stop || ktimer_check_dead(timer))) {
                /* 不能被销毁, 移动到新时间戳红黑树节点链表内 */
                timer->ms = ms + timer->intval;
                dlist_remove(timers, node);
                _ktimer_add_node(timer, node, timer->ms);
            }
        }
        /* 销毁红黑树节点 */
        krbtree_delete(timer_loop->timer_tree, rb_node);
        /* 查找时间戳最小节点 */
        rb_node = krbtree_min(timer_loop->timer_tree);
        if (!rb_node) {
            /* 没有节点 */
            break;
        }
        /* 获取最小节点的时间戳 */
        key = krbnode_get_key(rb_node);
    }
    return count;
}

ktimer_loop_t* ktimer_get_loop(ktimer_t* timer) {
    verify(timer);
    return timer->timer_loop;
}

ktimer_t* ktimer_create(ktimer_loop_t* timer_loop) {
    ktimer_t* timer = 0;
    verify(timer_loop);
    timer = create(ktimer_t);
    verify(timer);
    memset(timer, 0, sizeof(ktimer_t));
    timer->timer_loop = timer_loop;
    return timer;
}

void ktimer_destroy(ktimer_t* timer) {
    verify(timer);
    free(timer);
}

int ktimer_check_dead(ktimer_t* timer) {
    verify(timer);
    if (timer->type == ktimer_type_once) {
        return 1;
    } else if (timer->type == ktimer_type_times) {
        if (timer->times <= timer->current_times) {
            return 1;
        }
    }
    return 0;
}

time_t ktimer_loop_get_tick_intval(ktimer_loop_t* timer_loop) {
    verify(timer_loop);
    return timer_loop->freq;
}

int ktimer_stop(ktimer_t* timer) {
    verify(timer);
    timer->stop = 1;
    if (!timer->current_list) {
        /* 还未启动的定时器, 未放入定时器链表 */
        ktimer_destroy(timer);
    }
    return error_ok;
}

int ktimer_start(ktimer_t* timer, ktimer_cb_t cb, void* data, time_t ms) {
    verify(timer);
    verify(cb);
    verify(ms);
    verify(timer->timer_loop);
    if (timer->current_list) {
        return error_multiple_start;
    }
    if (error_ok != _ktimer_add(timer, ms)) {
        return error_fail;
    }
    timer->cb     = cb;
    timer->data   = data;
    timer->type   = ktimer_type_period;
    timer->ms     = time_get_milliseconds_19700101() + ms;
    timer->intval = ms;
    return error_ok;
}

int ktimer_start_once(ktimer_t* timer, ktimer_cb_t cb, void* data, time_t ms) {
    verify(timer);
    verify(cb);
    verify(ms);
    verify(timer->timer_loop);
    if (timer->current_list) {
        return error_multiple_start;
    }
    if (error_ok != _ktimer_add(timer, ms)) {
        return error_fail;
    }
    timer->cb     = cb;
    timer->data   = data;
    timer->type   = ktimer_type_once;
    timer->ms     = time_get_milliseconds_19700101() + ms;
    timer->intval = ms;
    return error_ok;
}

int ktimer_start_times(ktimer_t* timer, ktimer_cb_t cb, void* data, time_t ms, int times) {
    verify(timer);
    verify(cb);
    verify(ms);
    verify(timer->timer_loop);
    if (timer->current_list) {
        return error_multiple_start;
    }
    if (error_ok != _ktimer_add(timer, ms)) {
        return error_fail;
    }
    timer->cb     = cb;
    timer->data   = data;
    timer->type   = ktimer_type_times;
    timer->times  = times;
    timer->ms     = time_get_milliseconds_19700101() + ms;
    timer->intval = ms;
    return error_ok;
}
