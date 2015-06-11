#include <math.h>

#include "timer.h"
#include "list.h"
#include "misc.h"

struct _timer_t {
    dlist_t*      current_list;  /* 所属链表 */
    dlist_node_t* list_node;     /* 链表节点 */
    timer_loop_t* timer_loop;    /* 定时器循环 */
    timer_type_e  type;          /* 定时器类型 */
    timer_cb_t    cb;            /* 定时器回调 */
    void*         data;          /* 自定义数据 */
    time_t        ms;            /* 下次触发时间 */
    time_t        intval;        /* 定时器间隔 */
    int           times;         /* 触发最大次数 */
    int           current_times; /* 当前触发次数 */
};

struct _timer_loop_t {
    dlist_t** timer_wheels; /* 时间轮链表数组 */
    int       running;      /* 运行标志 */
    int       max_slot;     /* 时间轮数组长度 */
    int       slot;         /* 当前槽位 */
    time_t    last_tick;    /* 上一次调用循环的时间（毫秒） */
    time_t    tick_intval;  /* 槽位刻度间隔（毫秒） */
};

int _timer_loop_select_slot(timer_loop_t* timer_loop, time_t ms);
void _timer_loop_add_timer(timer_loop_t* timer_loop, timer_t* timer);
void _timer_loop_add_timer_node(timer_loop_t* timer_loop, dlist_node_t* node, time_t ms);
dlist_node_t* _timer_loop_remove_timer(timer_t* timer);

timer_loop_t* timer_loop_create(time_t freq, int slot) {
    int i = 0;
    timer_loop_t* timer_loop = create(timer_loop_t);
    assert(timer_loop);
    memset(timer_loop, 0, sizeof(timer_loop_t));
    timer_loop->max_slot     = slot;
    timer_loop->tick_intval  = freq;
    timer_loop->last_tick    = time_get_milliseconds();
    timer_loop->slot         = 1;
    timer_loop->timer_wheels = (dlist_t**)create_type(dlist_t, sizeof(dlist_t*) * timer_loop->max_slot);
    assert(timer_loop->timer_wheels);
    for (; i < timer_loop->max_slot; i++) {
        timer_loop->timer_wheels[i] = dlist_create();
        assert(timer_loop->timer_wheels[i]);
    }
    return timer_loop;
}

void timer_loop_destroy(timer_loop_t* timer_loop) {
    int i = 0;
    timer_t*      timer = 0;
    dlist_node_t* node  = 0;
    dlist_node_t* temp  = 0;
    assert(timer_loop);
    /* 销毁所有槽内链表 */
    for (; i < timer_loop->max_slot; i++) {
        dlist_for_each_safe(timer_loop->timer_wheels[i], node, temp) {
            timer = (timer_t*)dlist_node_get_data(node);
            timer_destroy(timer);
        }
        dlist_destroy(timer_loop->timer_wheels[i]);
    }
    destroy(timer_loop->timer_wheels);
    destroy(timer_loop);
}

void timer_loop_run(timer_loop_t* timer_loop) {
    timer_loop->running = 1;
    while (timer_loop->running) {
        thread_sleep_ms((int)timer_loop->tick_intval);
        timer_loop_run_once(timer_loop);
    }
}

void timer_loop_exit(timer_loop_t* timer_loop) {
    timer_loop->running = 0;
}

int _timer_loop_select_slot(timer_loop_t* timer_loop, time_t ms) {
    /* 计算按当前槽位的位置，后面哪个槽位会触发定时器 */
    return (int)(timer_loop->slot + ms / timer_loop->tick_intval) % timer_loop->max_slot;
}

void _timer_loop_add_timer(timer_loop_t* timer_loop, timer_t* timer) {
    /* 新timer都加入到下次运行的槽位，如果未过期会被调整到后续槽位 */
    dlist_node_t* node = dlist_add_tail_node(timer_loop->timer_wheels[timer_loop->slot], timer);
    timer_set_current_list(timer, timer_loop->timer_wheels[timer_loop->slot]);
    timer_set_current_list_node(timer, node);
}

void _timer_loop_add_timer_node(timer_loop_t* timer_loop, dlist_node_t* node, time_t ms) {
    int slot = _timer_loop_select_slot(timer_loop, ms);
    dlist_add_tail(timer_loop->timer_wheels[slot], node);
}

dlist_node_t* _timer_loop_remove_timer(timer_t* timer) {
    dlist_t* current_list = timer_get_current_list(timer);
    dlist_node_t* list_node = timer_get_current_list_node(timer);
    dlist_remove(current_list, list_node);
    return list_node;
} 

int timer_loop_run_once(timer_loop_t* timer_loop) {
    dlist_node_t* node   = 0;
    dlist_node_t* temp   = 0;
    dlist_t*      timers = 0;
    timer_t*      timer  = 0;
    time_t        ms     = time_get_milliseconds(); /* 当前时间戳（毫秒） */
    int           count  = 0;
    assert(timer_loop);
    timers = timer_loop->timer_wheels[timer_loop->slot];
    dlist_for_each_safe(timers, node, temp) {
        timer = (timer_t*)dlist_node_get_data(node);
        /* 处理定时器 */
        if (timer_check_timeout(timer, ms)) {
            count++;
        }
    }
    /* 下一个槽位 */
    timer_loop->slot = (timer_loop->slot + 1) % timer_loop->max_slot;
    /* 记录上次tick时间戳 */
    timer_loop->last_tick = ms;
    return count;
}

timer_loop_t* timer_get_loop(timer_t* timer) {
    return timer->timer_loop;
}

void timer_set_current_list(timer_t* timer, dlist_t* list) {
    assert(timer);
    assert(list);
    timer->current_list = list;
}

void timer_set_current_list_node(timer_t* timer, dlist_node_t* node) {
    assert(timer);
    assert(node);
    timer->list_node = node;
}

dlist_t* timer_get_current_list(timer_t* timer) {
    assert(timer);
    return timer->current_list;
}

dlist_node_t* timer_get_current_list_node(timer_t* timer) {
    assert(timer);
    return timer->list_node;
}

timer_t* timer_create(timer_loop_t* timer_loop) {
    timer_t* timer = 0;
    assert(timer_loop);
    timer = create(timer_t);
    assert(timer);
    memset(timer, 0, sizeof(timer_t));
    timer->timer_loop = timer_loop;
    return timer;
}

void timer_destroy(timer_t* timer) {
    assert(timer);
    assert(timer->current_list);
    assert(timer->list_node);
    dlist_delete(timer->current_list, timer->list_node);
    free(timer);
}

int timer_check_dead(timer_t* timer) {
    if (timer->type == timer_type_once) {
        return 1;
    } else if (timer->type == timer_type_times) {
        if (timer->times <= timer->current_times) {
            return 1;
        }
    }
    return 0;
}

time_t timer_loop_get_tick_intval(timer_loop_t* timer_loop) {
    return timer_loop->tick_intval;
}

int timer_check_timeout(timer_t* timer, time_t ms) {
    dlist_node_t* node        = 0;
    time_t        tick_intval = 0;
    timer_loop_t* timer_loop  = 0;
    assert(timer);
    timer_loop = timer->timer_loop;
    assert(timer_loop);
    tick_intval = timer_loop_get_tick_intval(timer_loop);
    if (timer->ms > ms) {
        /* 在将来的时间到期，检测是否可以触发 */
        if (timer->ms - ms >= tick_intval) {
            /* 距离下次超时时间大于（等于）一个tick间隔, 可以被下次触发, 调整到后续槽位槽位 */
            node = _timer_loop_remove_timer(timer);
            _timer_loop_add_timer_node(timer->timer_loop, node, timer->ms - ms);
            return 0;
        } else {
            /* 距离下次超时时间小于一个tick间隔, 如果下一轮触发已经多超时了一个tick间隔 */
        }
    }
    timer->cb(timer, timer->data);
    if (timer->type == timer_type_once) {
        timer_destroy(timer);
    } else if (timer->type == timer_type_period) {
        timer->ms = ms + timer->intval;
        node = _timer_loop_remove_timer(timer);
        _timer_loop_add_timer_node(timer->timer_loop, node, timer->intval);
    } else if (timer->type == timer_type_times) {
        timer->ms = ms + timer->intval;
        timer->current_times++;
        if (timer->times <= timer->current_times) {
            timer_destroy(timer);
        } else {
            node = _timer_loop_remove_timer(timer);
            _timer_loop_add_timer_node(timer->timer_loop, node, timer->intval);
        }
    }
    return 1;
}

int timer_stop(timer_t* timer) {
    assert(timer);
    timer_destroy(timer);
    return error_ok;
}

int timer_start(timer_t* timer, timer_cb_t cb, void* data, time_t ms) {
    assert(timer);
    assert(cb);
    assert(ms);
    timer->cb     = cb;
    timer->data   = data;
    timer->type   = timer_type_period;
    timer->ms     = time_get_milliseconds() + ms;
    timer->intval = ms;
    _timer_loop_add_timer(timer->timer_loop, timer);
    return error_ok;
}

int timer_start_once(timer_t* timer, timer_cb_t cb, void* data, time_t ms) {
    assert(timer);
    assert(cb);
    assert(ms);
    timer->cb     = cb;
    timer->data   = data;
    timer->type   = timer_type_once;
    timer->ms     = time_get_milliseconds() + ms;
    timer->intval = ms;
    _timer_loop_add_timer(timer->timer_loop, timer);
    return error_ok;
}

int timer_start_times(timer_t* timer, timer_cb_t cb, void* data, time_t ms, int times) {
    assert(timer);
    assert(cb);
    assert(ms);
    timer->cb     = cb;
    timer->data   = data;
    timer->type   = timer_type_times;
    timer->times  = times;
    timer->ms     = time_get_milliseconds() + ms;
    timer->intval = ms;
    _timer_loop_add_timer(timer->timer_loop, timer);
    return error_ok;
}
