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

#include "helper.h"
#include "knet.h"

static int Test_Timer_i = 0;

CASE(Test_Timer_Run_Once) {
    struct holder {        
        static void timer_cb(ktimer_t* t, void*) {
            Test_Timer_i++;
            EXPECT_TRUE(ktimer_check_dead(t));
        }
    };

    ktimer_loop_t* l = ktimer_loop_create(1000, 128);
    for (int i = 0; i < 100; i++) {
        ktimer_t* t = ktimer_create(l);
        // 周期为1000毫秒，只运行一次的定时器
        ktimer_start_once(t, &holder::timer_cb, 0, 1000);
    }
    thread_sleep_ms(1000);
    ktimer_loop_run_once(l);
    EXPECT_TRUE(100 == Test_Timer_i);
    ktimer_loop_destroy(l);
}

CASE(Test_Timer_Run) {
    Test_Timer_i = 0;
    struct holder {        
        static void timer_cb(ktimer_t* t, void*) {
            EXPECT_FALSE(ktimer_check_dead(t));
            Test_Timer_i++;
            if (Test_Timer_i >= 100) {
                ktimer_loop_exit(ktimer_get_loop(t));
            }
        }
    };

    ktimer_loop_t* l = ktimer_loop_create(1000, 128);
    for (int i = 0; i < 100; i++) {
        ktimer_t* t = ktimer_create(l);
        // 周期为1000毫秒的定时器
        ktimer_start(t, &holder::timer_cb, 0, 1000);
    }
    ktimer_loop_run(l);
    EXPECT_TRUE(100 == Test_Timer_i);
    ktimer_loop_destroy(l);
}

CASE(Test_Timer_Start_Times) {
    Test_Timer_i = 0;
    struct holder {        
        static void timer_cb(ktimer_t* t, void*) {
            Test_Timer_i++;
            if (Test_Timer_i >= 200) {
                ktimer_loop_exit(ktimer_get_loop(t));
            }
        }
    };

    ktimer_loop_t* l = ktimer_loop_create(1000, 128);
    for (int i = 0; i < 100; i++) {
        ktimer_t* t = ktimer_create(l);
        // 周期为1000毫秒的定时器, 运行2次
        ktimer_start_times(t, &holder::timer_cb, 0, 1000, 2);
    }
    ktimer_loop_run(l);
    EXPECT_TRUE(200 == Test_Timer_i);
    ktimer_loop_destroy(l);
}

CASE(Test_Timer_Stop) {
    Test_Timer_i = 0;
    struct holder {        
        static void timer_cb(ktimer_t* t, void*) {
            EXPECT_TRUE(error_ok == ktimer_stop(t));
            Test_Timer_i++;
            if (Test_Timer_i >= 100) {
                ktimer_loop_exit(ktimer_get_loop(t));
            }
        }
    };

    ktimer_loop_t* l = ktimer_loop_create(1000, 128);
    for (int i = 0; i < 100; i++) {
        ktimer_t* t = ktimer_create(l);
        // 周期为1000毫秒的定时器
        ktimer_start(t, &holder::timer_cb, 0, 1000);
    }
    ktimer_loop_run(l);
    EXPECT_TRUE(100 == Test_Timer_i);
    ktimer_loop_destroy(l);
}
