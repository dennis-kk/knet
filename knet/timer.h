#ifndef TIMER_H
#define TIMER_H

#include "config.h"

timer_loop_t* timer_loop_create(time_t tick);
void timer_loop_destroy(timer_loop_t* timer_loop);
int timer_loop_run_once(timer_loop_t* timer_loop);
int timer_loop_exit(timer_loop_t* timer_loop);

timer_t* timer_create(timer_loop_t* timer_loop);
void timer_destroy(timer_t* timer);
int timer_stop(timer_t* timer);

int timer_start(timer_t* timer, timer_cb_t* cb, void* data, time_t ms);
int timer_start_once(timer_t* timer, timer_cb_t* cb, void* data, time_t ms);
int timer_start_times(timer_t* timer, timer_cb_t* cb, void* data, time_t ms, int times);

#endif /* TIMER_H */
