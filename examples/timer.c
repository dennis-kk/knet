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
	printf("timer timeout\n");
	times++;
	if (times >= MAX_TIMES) {
		timer_stop(timer);
	}
}

int main() {
	int i = 0;
	timer_loop_t* loop = timer_loop_create(1000, 128);
	timer_t* timer = timer_create(loop);
	timer_start_once(timer, timer_once_cb, 0, 1000);
	thread_sleep_ms(1000);
	timer_loop_run_once(loop);
	/* 定时器已经被销毁了，重新建立一个新的定时器 */
	timer = timer_create(loop);
	timer_start(timer, timer_cb, 0, 1000);
	for (; i < MAX_TIMES; i++) {
		thread_sleep_ms(1000);
		timer_loop_run_once(loop);
	}
	return 0;
}

#endif /* COMPILE_TIMER */
