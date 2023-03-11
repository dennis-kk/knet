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

#include "timer.h"
#include "list.h"
#include "misc.h"
#include "logger.h"
#include "rb_tree.h"

/**
 * ��ʱ��
 */
struct _ktimer_t {
    kdlist_t*       current_list;  /* �������� */
    kdlist_node_t*  list_node;     /* ����ڵ� */
    ktimer_loop_t*  timer_loop;    /* ��ʱ��ѭ�� */
    ktimer_type_e   type;          /* ��ʱ������ */
    ktimer_cb_t     cb;            /* ��ʱ���ص� */
    void*           data;          /* �Զ������� */
    uint64_t        ms;            /* �´δ���ʱ�� */
    uint64_t        intval;        /* ��ʱ����� */
    int             times;         /* ���������� */
    int             current_times; /* ��ǰ�������� */
    int             stop;          /* ��ֹ��־ */
};

/**
 * ��ʱ��ѭ��
 */
struct _ktimer_loop_t {
    krbtree_t* timer_tree; /* ����� */
    int        running;    /* ���б�־ */
    uint64_t   last_tick;  /* ��һ�ε���ѭ����ʱ�䣨���룩 */
    uint64_t   freq;       /* ѭ�����ü��(����) */
};

/**
 * ���ٺ�����ڵ�
 */
void _rb_node_destroy_cb(void* ptr, uint64_t key);

/**
 * ��Ӷ�ʱ��
 * @param timer ��ʱ��
 * @param node ��ʱ������ڵ�
 * @param ms ����ʱ���(����)
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int _ktimer_add_node(ktimer_t* timer, kdlist_node_t* node, time_t ms);

/**
 * ��Ӷ�ʱ��
 * @param timer ��ʱ��
 * @param ms ����ʱ���(����)
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int _ktimer_add(ktimer_t* timer, time_t ms);

void _rb_node_destroy_cb(void* ptr, uint64_t key) {
    kdlist_node_t* node = 0;
    kdlist_node_t* temp = 0;
    kdlist_t*      list = (kdlist_t*)ptr;
    (void)key;
    /* ���������ڵĶ�ʱ�� */
    dlist_for_each_safe(list, node, temp) {
        ktimer_t* timer = (ktimer_t*)dlist_node_get_data(node);
        ktimer_destroy(timer);
    }
    /* �������� */
    dlist_destroy(list);
}

int _ktimer_add_node(ktimer_t* timer, kdlist_node_t* node, time_t ms) {
    kdlist_t*  list    = 0;
    krbnode_t* rb_node = 0;
    /* ���ҽڵ� */
    rb_node = krbtree_find(timer->timer_loop->timer_tree, ms);
    if (!rb_node) { /* δ�ҵ��ڵ� */
        list    = dlist_create();
        /* ����������ڵ� */
        rb_node = krbnode_create(ms, list, _rb_node_destroy_cb);
        /* ���������ڵ� */
        krbtree_insert(timer->timer_loop->timer_tree, rb_node);
    }
    if (rb_node) {
        /* ���ö�ʱ���������� */
        timer->current_list = (kdlist_t*)krbnode_get_ptr(rb_node);
        /* ��ӵ�����β�� */
        dlist_add_tail(timer->current_list, node);
        /* �����ڵĽڵ� */
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
        /* ���������ڵ� */
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
    ktimer_loop_t* timer_loop = knet_create(ktimer_loop_t);
    verify(timer_loop);
    memset(timer_loop, 0, sizeof(ktimer_loop_t));
    timer_loop->last_tick  = time_get_milliseconds_19700101();
    timer_loop->timer_tree = krbtree_create();
    timer_loop->freq       = freq;
    return timer_loop;
}

void ktimer_loop_destroy(ktimer_loop_t* timer_loop) {
    verify(timer_loop);
    /* ���ٺ���� */
    krbtree_destroy(timer_loop->timer_tree);
    knet_free(timer_loop);
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
    uint64_t       ms      = time_get_milliseconds_19700101(); /* ��ǰʱ��������룩 */
    int            count   = 0;
    verify(timer_loop);
    /* ��¼�ϴ�tickʱ��� */
    timer_loop->last_tick = ms;
    /* ����ʱ�����С�ڵ� */
    rb_node = krbtree_min(timer_loop->timer_tree);
    if (!rb_node) {
        /* û�нڵ� */
        return 0;
    }
    /* ��ȡ��С�ڵ��ʱ��� */
    key = krbnode_get_key(rb_node);
    while (rb_node && (key < ms)) { /* ���� */
        /* ��ȡ��ǰʱ����Ķ�ʱ������ */
        timers = (kdlist_t*)krbnode_get_ptr(rb_node);
        verify(timers);
        /* �������е��ڵĶ�ʱ�� */
        dlist_for_each_safe(timers, node, temp) {
            timer = (ktimer_t*)dlist_node_get_data(node);
            verify(timer);
            if (!timer->stop) {
                if (timer->cb) {
                    /* ��ʱ������,���ûص� */
                    timer->cb(timer, timer->data);
                    count += 1;
                    /* ���ô������� */
                    timer->current_times += 1;
                }
                if (!ktimer_check_dead(timer)) {
                    /* ���ܱ�����, �ƶ�����ʱ���������ڵ������� */
                    timer->ms = ms + timer->intval;
                    dlist_remove(timers, node);
                    _ktimer_add_node(timer, node, timer->ms);
                }
            }
        }
        /* ���ٺ�����ڵ� */
        krbtree_delete(timer_loop->timer_tree, rb_node);
        /* ����ʱ�����С�ڵ� */
        rb_node = krbtree_min(timer_loop->timer_tree);
        if (!rb_node) {
            /* û�нڵ� */
            break;
        }
        /* ��ȡ��С�ڵ��ʱ��� */
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
    timer = knet_create(ktimer_t);
    verify(timer);
    memset(timer, 0, sizeof(ktimer_t));
    timer->timer_loop = timer_loop;
    return timer;
}

void ktimer_destroy(ktimer_t* timer) {
    verify(timer);
    knet_free(timer);
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
        /* ��δ�����Ķ�ʱ��, δ���붨ʱ������ */
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
