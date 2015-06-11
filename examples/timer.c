#include "example_config.h"

#if COMPILE_TIMER

#include "knet.h"
#include "misc.h"

void ktimer_once_cb(kktimer_t* timer, void* data) {
    printf("timer timeout\n");
}

#define MAX_TIMES 5
int times = 0;
void ktimer_cb(kktimer_t* timer, void* data) {
    ktimer_loop_t* loop = ktimer_get_loop(timer);
    printf("timer timeout\n");
    times++;
    if (times >= MAX_TIMES) {
        ktimer_stop(timer);
        ktimer_loop_exit(loop);
    }
}

int main() {
    ktimer_loop_t* loop = ktimer_loop_create(1000, 128);
    kktimer_t* ktimer_once = ktimer_create(loop);
    kktimer_t* ktimer_period = ktimer_create(loop);
    ktimer_start_once(ktimer_once, ktimer_once_cb, 0, 1000);
    ktimer_start(ktimer_period, ktimer_cb, 0, 1000);
    ktimer_loop_run(loop);
    return 0;
}

#endif /* COMPILE_TIMER */
