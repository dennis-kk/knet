#include "timer.h"
#include "list.h"
#include "misc.h"

struct _ktimer_t {
    dlist_t*       current_list;  /* 所属链表 */
    dlist_node_t*  list_node;     /* 链表节点 */
    ktimer_loop_t* ktimer_loop;    /* 定时器循环 */
    ktimer_type_e  type;          /* 定时器类型 */
    ktimer_cb_t    cb;            /* 定时器回调 */
    void*          data;          /* 自定义数据 */
    time_t         ms;            /* 下次触发时间 */
    time_t         intval;        /* 定时器间隔 */
    int            times;         /* 触发最大次数 */
    int            current_times; /* 当前触发次数 */
    int            stop;          /* 终止标志 */
};

struct _ktimer_loop_t {
    dlist_t** ktimer_wheels; /* 时间轮链表数组 */
    int       running;      /* 运行标志 */
    int       max_slot;     /* 时间轮数组长度 */
    int       slot;         /* 当前槽位 */
    time_t    last_tick;    /* 上一次调用循环的时间（毫秒） */
    time_t    tick_intval;  /* 槽位刻度间隔（毫秒） */
};

int _ktimer_loop_select_slot(ktimer_loop_t* ktimer_loop, time_t ms);
void _ktimer_loop_add_timer(ktimer_loop_t* ktimer_loop, ktimer_t* timer);
void _ktimer_loop_add_ktimer_node(ktimer_loop_t* ktimer_loop, dlist_node_t* node, time_t ms);
dlist_node_t* _ktimer_loop_remove_timer(ktimer_t* timer);
int _ktimer_check_stop(ktimer_t* timer);

ktimer_loop_t* ktimer_loop_create(time_t freq, int slot) {
    int i = 0;
    ktimer_loop_t* ktimer_loop = create(ktimer_loop_t);
    assert(ktimer_loop);
    assert(freq);
    assert(slot);
    memset(ktimer_loop, 0, sizeof(ktimer_loop_t));
    ktimer_loop->max_slot     = slot;
    ktimer_loop->tick_intval  = freq;
    ktimer_loop->last_tick    = time_get_milliseconds();
    ktimer_loop->slot         = 1;
    ktimer_loop->ktimer_wheels = (dlist_t**)create_type(dlist_t, sizeof(dlist_t*) * ktimer_loop->max_slot);
    assert(ktimer_loop->ktimer_wheels);
    for (; i < ktimer_loop->max_slot; i++) {
        ktimer_loop->ktimer_wheels[i] = dlist_create();
        assert(ktimer_loop->ktimer_wheels[i]);
    }
    return ktimer_loop;
}

void ktimer_loop_destroy(ktimer_loop_t* ktimer_loop) {
    int i = 0;
    ktimer_t*      timer = 0;
    dlist_node_t* node  = 0;
    dlist_node_t* temp  = 0;
    assert(ktimer_loop);
    /* 销毁所有槽内链表 */
    for (; i < ktimer_loop->max_slot; i++) {
        dlist_for_each_safe(ktimer_loop->ktimer_wheels[i], node, temp) {
            timer = (ktimer_t*)dlist_node_get_data(node);
            ktimer_destroy(timer);
        }
        dlist_destroy(ktimer_loop->ktimer_wheels[i]);
    }
    destroy(ktimer_loop->ktimer_wheels);
    destroy(ktimer_loop);
}

void ktimer_loop_run(ktimer_loop_t* ktimer_loop) {
    assert(ktimer_loop);
    ktimer_loop->running = 1;
    while (ktimer_loop->running) {
        thread_sleep_ms((int)ktimer_loop->tick_intval);
        ktimer_loop_run_once(ktimer_loop);
    }
}

void ktimer_loop_exit(ktimer_loop_t* ktimer_loop) {
    assert(ktimer_loop);
    ktimer_loop->running = 0;
}

int _ktimer_loop_select_slot(ktimer_loop_t* ktimer_loop, time_t ms) {
    /* 计算按当前槽位的位置，后面哪个槽位会触发定时器 */
    return (int)(ktimer_loop->slot + ms / ktimer_loop->tick_intval) % ktimer_loop->max_slot;
}

void _ktimer_loop_add_timer(ktimer_loop_t* ktimer_loop, ktimer_t* timer) {
    /* 新timer都加入到下次运行的槽位，如果未过期会被调整到后续槽位 */
    dlist_node_t* node = dlist_add_tail_node(ktimer_loop->ktimer_wheels[ktimer_loop->slot], timer);
    ktimer_set_current_list(timer, ktimer_loop->ktimer_wheels[ktimer_loop->slot]);
    ktimer_set_current_list_node(timer, node);
}

void _ktimer_loop_add_ktimer_node(ktimer_loop_t* ktimer_loop, dlist_node_t* node, time_t ms) {
    int slot = _ktimer_loop_select_slot(ktimer_loop, ms);
    ktimer_t* timer = (ktimer_t*)dlist_node_get_data(node);
    dlist_add_tail(ktimer_loop->ktimer_wheels[slot], node);
    ktimer_set_current_list(timer, ktimer_loop->ktimer_wheels[slot]);
}

dlist_node_t* _ktimer_loop_remove_timer(ktimer_t* timer) {
    dlist_t* current_list = ktimer_get_current_list(timer);
    dlist_node_t* list_node = ktimer_get_current_list_node(timer);
    dlist_remove(current_list, list_node);
    ktimer_set_current_list(timer, 0);
    return list_node;
} 

int ktimer_loop_run_once(ktimer_loop_t* ktimer_loop) {
    dlist_node_t* node   = 0;
    dlist_node_t* temp   = 0;
    dlist_t*      timers = 0;
    ktimer_t*      timer  = 0;
    time_t        ms     = time_get_milliseconds(); /* 当前时间戳（毫秒） */
    int           count  = 0;
    assert(ktimer_loop);
    timers = ktimer_loop->ktimer_wheels[ktimer_loop->slot];
    dlist_for_each_safe(timers, node, temp) {
        timer = (ktimer_t*)dlist_node_get_data(node);
        if (!_ktimer_check_stop(timer)) {
            /* 处理定时器 */
            if (ktimer_check_timeout(timer, ms)) {
                count++;
            }
        } else {
            /* 销毁 */
            ktimer_destroy(timer);
        }
    }
    /* 下一个槽位 */
    ktimer_loop->slot = (ktimer_loop->slot + 1) % ktimer_loop->max_slot;
    /* 记录上次tick时间戳 */
    ktimer_loop->last_tick = ms;
    return count;
}

int _ktimer_check_stop(ktimer_t* timer) {
    return timer->stop;
}

ktimer_loop_t* ktimer_get_loop(ktimer_t* timer) {
    return timer->ktimer_loop;
}

void ktimer_set_current_list(ktimer_t* timer, dlist_t* list) {
    assert(timer); /* list可以为零 */
    timer->current_list = list;
}

void ktimer_set_current_list_node(ktimer_t* timer, dlist_node_t* node) {
    assert(timer);
    assert(node);
    timer->list_node = node;
}

dlist_t* ktimer_get_current_list(ktimer_t* timer) {
    assert(timer);
    return timer->current_list;
}

dlist_node_t* ktimer_get_current_list_node(ktimer_t* timer) {
    assert(timer);
    return timer->list_node;
}

ktimer_t* ktimer_create(ktimer_loop_t* ktimer_loop) {
    ktimer_t* timer = 0;
    assert(ktimer_loop);
    timer = create(ktimer_t);
    assert(timer);
    memset(timer, 0, sizeof(ktimer_t));
    timer->ktimer_loop = ktimer_loop;
    return timer;
}

void ktimer_destroy(ktimer_t* timer) {
    assert(timer);
    assert(timer->current_list);
    assert(timer->list_node);
    dlist_delete(timer->current_list, timer->list_node);
    free(timer);
}

int ktimer_check_dead(ktimer_t* timer) {
    if (timer->type == ktimer_type_once) {
        return 1;
    } else if (timer->type == ktimer_type_times) {
        if (timer->times <= timer->current_times) {
            return 1;
        }
    }
    return 0;
}

time_t ktimer_loop_get_tick_intval(ktimer_loop_t* ktimer_loop) {
    return ktimer_loop->tick_intval;
}

int ktimer_check_timeout(ktimer_t* timer, time_t ms) {
    dlist_node_t* node        = 0;
    time_t        tick_intval = 0;
    ktimer_loop_t* ktimer_loop  = 0;
    assert(timer);
    ktimer_loop = timer->ktimer_loop;
    assert(ktimer_loop);
    tick_intval = ktimer_loop_get_tick_intval(ktimer_loop);
    if (timer->ms > ms) {
        /* 在将来的时间到期，检测是否可以触发 */
        if (timer->ms - ms >= tick_intval) {
            /* 距离下次超时时间大于（等于）一个tick间隔, 可以被下次触发, 调整到后续槽位槽位 */
            node = _ktimer_loop_remove_timer(timer);
            _ktimer_loop_add_ktimer_node(timer->ktimer_loop, node, timer->ms - ms);
            return 0;
        } else {
            /* 距离下次超时时间小于一个tick间隔, 如果下一轮触发已经多超时了一个tick间隔 */
        }
    }
    if (timer->type == ktimer_type_times) {
        /* 先改变次数 */
        timer->ms = ms + timer->intval;
        timer->current_times++;
    }
    timer->cb(timer, timer->data);
    if (_ktimer_check_stop(timer)) {
        /* 回调内调用ktimer_stop() */
        ktimer_destroy(timer);
        return 1;
    }
    if (timer->type == ktimer_type_once) {
        ktimer_destroy(timer);
    } else if (timer->type == ktimer_type_period) {
        timer->ms = ms + timer->intval;
        node = _ktimer_loop_remove_timer(timer);
        _ktimer_loop_add_ktimer_node(timer->ktimer_loop, node, timer->intval);
    } else if (timer->type == ktimer_type_times) {
        if (timer->times <= timer->current_times) {
            ktimer_destroy(timer);
        } else {
            node = _ktimer_loop_remove_timer(timer);
            _ktimer_loop_add_ktimer_node(timer->ktimer_loop, node, timer->intval);
        }
    }
    return 1;
}

int ktimer_stop(ktimer_t* timer) {
    assert(timer);
    timer->stop = 1;
    return error_ok;
}

int ktimer_start(ktimer_t* timer, ktimer_cb_t cb, void* data, time_t ms) {
    assert(timer);
    assert(cb);
    assert(ms);
    if (timer->current_list) {
        return error_multiple_start;
    }
    timer->cb     = cb;
    timer->data   = data;
    timer->type   = ktimer_type_period;
    timer->ms     = time_get_milliseconds() + ms;
    timer->intval = ms;
    _ktimer_loop_add_timer(timer->ktimer_loop, timer);
    return error_ok;
}

int ktimer_start_once(ktimer_t* timer, ktimer_cb_t cb, void* data, time_t ms) {
    assert(timer);
    assert(cb);
    assert(ms);
    if (timer->current_list) {
        return error_multiple_start;
    }
    timer->cb     = cb;
    timer->data   = data;
    timer->type   = ktimer_type_once;
    timer->ms     = time_get_milliseconds() + ms;
    timer->intval = ms;
    _ktimer_loop_add_timer(timer->ktimer_loop, timer);
    return error_ok;
}

int ktimer_start_times(ktimer_t* timer, ktimer_cb_t cb, void* data, time_t ms, int times) {
    assert(timer);
    assert(cb);
    assert(ms);
    if (timer->current_list) {
        return error_multiple_start;
    }
    timer->cb     = cb;
    timer->data   = data;
    timer->type   = ktimer_type_times;
    timer->times  = times;
    timer->ms     = time_get_milliseconds() + ms;
    timer->intval = ms;
    _ktimer_loop_add_timer(timer->ktimer_loop, timer);
    return error_ok;
}
