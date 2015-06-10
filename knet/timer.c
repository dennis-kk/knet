#include "timer.h"
#include "list.h"
#include "misc.h"

/*
 * WARNING 未经测试，需要等测试完成
 */

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
    int       max_slot;     /* 时间轮数组长度 */
    int       slot;         /* 当前槽位 */
    time_t    last_tick;    /* 上一次调用循环的时间（毫秒） */
    time_t    tick_intval;  /* 槽位刻度间隔（毫秒） */
};

timer_loop_t* timer_loop_create(time_t freq, int slot) {
    int i = 0;
    timer_loop_t* timer_loop = create(timer_loop_t);
    assert(timer_loop);
    memset(timer_loop, 0, sizeof(timer_loop_t));
    timer_loop->max_slot     = slot;
    timer_loop->tick_intval  = freq;
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

void timer_loop_add_timer(timer_loop_t* timer_loop, timer_t* timer, time_t ms) {
    int slot = (int)(timer_loop->slot + ms / timer_loop->tick_intval) % timer_loop->max_slot;
    dlist_node_t* node = dlist_add_tail_node(timer_loop->timer_wheels[slot], timer);
    timer_set_current_list(timer, timer_loop->timer_wheels[slot]);
    timer_set_current_list_node(timer, node);
}

int timer_loop_run_once(timer_loop_t* timer_loop) {
    dlist_node_t* node   = 0;
    dlist_node_t* temp   = 0;
    dlist_t*      timers = 0;
    timer_t*      timer  = 0;
    int           i      = 0;
    time_t        ms     = time_get_milliseconds(); /* 当前时间戳（毫秒） */
    int           slots  = 0;                       /* 需要走的槽位数 */
    int           count  = 0;
    assert(timer_loop);
    if (!timer_loop->last_tick) {
        timer_loop->last_tick = ms;
    } else {
        slots = (int)((ms - timer_loop->last_tick) / timer_loop->tick_intval);        
    }
    for (; i < slots; i++) {
        timer_loop->slot = (timer_loop->slot + 1) % timer_loop->max_slot;
        timers = timer_loop->timer_wheels[timer_loop->slot];
        dlist_for_each_safe(timers, node, temp) {
            timer = (timer_t*)dlist_node_get_data(node);
            /* 处理定时器 */
            if (timer_check_timeout(timer, ms)) {
                count++;
            }
        }
    }
    return count;
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

int timer_check_timeout(timer_t* timer, time_t ms) {
    assert(timer);
    if (timer->ms > ms) {
        return 0;
    }
    timer->cb(timer, timer->data);
    if (timer->type == timer_type_once) {
        timer_destroy(timer);
    } else if (timer->type == timer_type_period) {
        timer->ms = ms + timer->intval;
    } else if (timer->type == timer_type_times) {
        timer->ms = ms + timer->intval;
        timer->current_times++;
        if (timer->times <= timer->current_times) {
            timer_destroy(timer);
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
    timer_loop_add_timer(timer->timer_loop, timer, ms);
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
    timer_loop_add_timer(timer->timer_loop, timer, ms);
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
    timer_loop_add_timer(timer->timer_loop, timer, ms);
    return error_ok;
}
