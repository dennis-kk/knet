#include "example_config.h"

#if COMPILE_TIMER

#include "knet.h"
#include "misc.h"

void timer_once_cb(timer_t* timer, void* data) {
	printf("timer timeout\n");
}

#define MAX_TIMES 5
int times = 0;
void timer_cb(timer_t* timer, void* data) {
    timer_loop_t* loop = timer_get_loop(timer);
	printf("timer timeout\n");
	times++;
	if (times >= MAX_TIMES) {
		timer_stop(timer);
        timer_loop_exit(loop);
	}
}

int main() {
	int i = 0;
	timer_loop_t* loop = timer_loop_create(1000, 128);
	timer_t* timer_once = timer_create(loop);
    timer_t* timer_period = timer_create(loop);
	timer_start_once(timer_once, timer_once_cb, 0, 1000);
	timer_start(timer_period, timer_cb, 0, 1000);
	timer_loop_run(loop);
	return 0;
}

#endif /* COMPILE_TIMER */
