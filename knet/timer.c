#include "timer.h"
#include "list.h"
#include "misc.h"

typedef enum timer_type_e {
    timer_type_once = 1,
    timer_type_period = 2,
    timer_type_times = 3,
};

struct _timer_t {
    dlist_t* current_list;
    dlist_node_t* list_node;
    timer_loop_t* timer_loop;
    timer_type_e type;
    timer_cb_t* cb;
    void* data;
    time_t ms;
    int times;
};

struct _timer_loop_t {
    dlist_t* timer_wheels;
    dlist_t* current_wheel;
    int wheel_count;
    time_t last_tick;
    time_t tick_intval;
};

timer_loop_t* timer_loop_create(time_t tick) {
    int i = 0;
    timer_loop_t* timer_loop = (timer_loop_t*)malloc(sizeof(timer_loop_t));
    assert(timer_loop);
    timer_loop->last_tick = time_get_milliseconds();
    timer_loop->tick_intval = tick;
    timer_loop->wheel_count = 256;
    timer_loop->timer_wheels = (dlist_t*)malloc(sizeof(dlist_t*) * timer_loop->wheel_count);
    assert(timer_loop->timer_wheels);
    for (; i < timer_loop->wheel_count; i++) {
        timer_loop->timer_wheels[i] = dlist_create();
        assert(timer_loop->timer_wheels[i]);
    }
    return timer_loop;
}

void timer_loop_destroy(timer_loop_t* timer_loop);
int timer_loop_run_once(timer_loop_t* timer_loop);
int timer_loop_exit(timer_loop_t* timer_loop);

timer_t* timer_create(timer_loop_t* timer_loop);
void timer_destroy(timer_t* timer);
int timer_stop(timer_t* timer);

int timer_start(timer_t* timer, timer_cb_t* cb, void* data, time_t ms) {
    time_t wheels = ms / tick;
}

int timer_start_once(timer_t* timer, timer_cb_t* cb, void* data, time_t ms);
int timer_start_times(timer_t* timer, timer_cb_t* cb, void* data, time_t ms, int times);
