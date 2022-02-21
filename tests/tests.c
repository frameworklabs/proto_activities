/* tests.c */

/* Includes */

#include <proto_activities.h>

#include <stdio.h>
#include <assert.h>

/* Helpers */

pa_activity (Delay, pa_ctx(unsigned remaining), unsigned i) {
    pa_self.remaining = i;
    while (pa_self.remaining-- > 0) {
        pa_pause;
    }
} pa_end;

pa_activity (CountDown, pa_ctx(unsigned remaining), unsigned i, unsigned* value) {
    pa_self.remaining = i;
    while (pa_self.remaining-- > 0) {
        *value = pa_self.remaining;
        pa_pause;
    }
} pa_end;

pa_activity (Counter, pa_ctx(unsigned value), unsigned* res) {
    pa_always {
        *res = pa_self.value++;
    } pa_always_end;
} pa_end;

/* Await Tests */

pa_activity (TestAwaitSpec, pa_ctx(), int* value, int* expected) {
    *value = 0;
    *expected = 1;
    pa_pause;
    pa_pause;
    *value = 99;
    pa_pause;
    *value = 42;
    *expected = 2;
    pa_pause;
    pa_pause;
    *value = 100;
    *expected = 3;
} pa_end;

pa_activity (TestAwaitTest, pa_ctx(), int value, int* actual) {
    *actual = 1;
    pa_pause;
    pa_await (value == 42);
    *actual = 2;
    pa_await (value == 100);
    *actual = 3;
} pa_end;

pa_activity (TestAwaitCheck, pa_ctx(), int actual, int expected) {
    pa_always {
        assert(actual == expected);
    } pa_always_end;
} pa_end;

pa_activity (TestAwait, pa_ctx(pa_co_res(4); int value; int actual; int expected;
                               pa_use(TestAwaitSpec); pa_use(TestAwaitTest); pa_use(TestAwaitCheck))) {
    pa_co(3) {
        pa_with_weak (TestAwaitSpec, &pa_self.value, &pa_self.expected);
        pa_with (TestAwaitTest, pa_self.value, &pa_self.actual);
        pa_with_weak (TestAwaitCheck, pa_self.actual, pa_self.expected);
    } pa_co_end;
} pa_end;

/* Run Tests */

pa_activity (TestRunSpec, pa_ctx(), int* value, int* expected) {
    *expected = 1;
    pa_pause;
    *expected = 0;
    pa_pause;
    *expected = 42;
    pa_pause;

    /* Test that activity is starting clean on second invocation. */
    *expected = 0;
    pa_pause;
    *expected = 42;
    pa_pause;
    *expected = 1;
} pa_end;

pa_activity (TestRunActivity, pa_ctx(int value), int* actual) {
    *actual = pa_self.value;
    pa_pause;
    pa_self.value = 42;
    *actual = pa_self.value;
    pa_pause;
} pa_end;

pa_activity (TestRunTest, pa_ctx(pa_use(TestRunActivity)), int value, int* actual) {
    *actual = 1;
    pa_pause;
    pa_run (TestRunActivity, actual);
    
    /* Test that activity is starting clean on second invocation. */
    *actual = 1;
    pa_run (TestRunActivity, actual);
    *actual = 1;
} pa_end;

pa_activity (TestRunCheck, pa_ctx(), int actual, int expected) {
    pa_always {
        assert(actual == expected);
    } pa_always_end;
} pa_end;

pa_activity (TestRun, pa_ctx(pa_co_res(4); int value; int actual; int expected;
                               pa_use(TestRunSpec); pa_use(TestRunTest); pa_use(TestRunCheck))) {
    pa_co(3) {
        pa_with_weak (TestRunSpec, &pa_self.value, &pa_self.expected);
        pa_with (TestRunTest, pa_self.value, &pa_self.actual);
        pa_with_weak (TestRunCheck, pa_self.actual, pa_self.expected);
    } pa_co_end;
} pa_end;

/* Co Tests */

pa_activity (TestCoSpec, pa_ctx(), int* value, int* expected) {
    /* All strong */
    *expected = 1;
    pa_pause;
    pa_pause;
    pa_pause;
    *expected = 2;

    /* All weak */
    pa_pause;
    *expected = 3;

    /* All but one weak */
    pa_pause;
    pa_pause;
    *expected = 4;
} pa_end;

pa_activity (TestCoTest, pa_ctx(pa_co_res(4);
                                pa_use_as(Delay, Delay1); pa_use_as(Delay, Delay2); pa_use_as(Delay, Delay3)),
                         int value, int* actual) {
    /* All strong */
    *actual = 1;
    pa_co(3) {
        pa_with_as (Delay, Delay1, 1);
        pa_with_as (Delay, Delay2, 3);
        pa_with_as (Delay, Delay3, 2);
    } pa_co_end;
    *actual = 2;

    /* All weak */
    pa_co(3) {
        pa_with_weak_as (Delay, Delay1, 1);
        pa_with_weak_as (Delay, Delay2, 3);
        pa_with_weak_as (Delay, Delay3, 2);
    } pa_co_end;
    *actual = 3;

    /* All but one weak */
    pa_co(3) {
        pa_with_weak_as (Delay, Delay1, 1);
        pa_with_weak_as (Delay, Delay2, 3);
        pa_with_as (Delay, Delay3, 2);
    } pa_co_end;
    *actual = 4;
} pa_end;

pa_activity (TestCoCheck, pa_ctx(), int actual, int expected) {
    pa_always {
        assert(actual == expected);
    } pa_always_end;
} pa_end;

pa_activity (TestCo, pa_ctx(pa_co_res(4); int value; int actual; int expected;
                            pa_use(TestCoSpec); pa_use(TestCoTest); pa_use(TestCoCheck))) {
    pa_co(3) {
        pa_with_weak (TestCoSpec, &pa_self.value, &pa_self.expected);
        pa_with (TestCoTest, pa_self.value, &pa_self.actual);
        pa_with_weak (TestCoCheck, pa_self.actual, pa_self.expected);
    } pa_co_end;
} pa_end;

/* When-Abort Tests */

pa_activity (TestWhenAbortSpec, pa_ctx(), int* value, int* expected) {

    /* Test that preemption happens only when conditiopn is true. */
    *expected = 0;
    pa_pause;
    *expected = 1;
    pa_pause;
    *expected = 2;
    pa_pause;
    *value = 42;
    *expected = -2;
    pa_pause;

    /* Test that Counter was cleared on abort (i.e. starts with 0 again) */
    *value = 0;
    *expected = 0;
    pa_pause;
    *value = 42;
    *expected = -2;
    pa_pause;

    /* Test condition immediately true. */
    *value = 42;
    *expected = 0;
    pa_pause;
    *expected = -2;
    pa_pause;
    
    /* Test completion when condition will not become true. */
    *value = 0;
    *expected = -1;
    pa_pause;
    pa_pause;
    *expected = -2;
} pa_end;

pa_activity (TestWhenAbortTest, pa_ctx(pa_use(Counter); pa_use(Delay)), int value, int* actual) {
    /* Test that preemption happens only when conditiopn is true. */
    *actual = -1;
    pa_when_abort (value == 42, Counter, (unsigned*)actual);
    *actual = -2;
    pa_pause;
    
    /* Test that Counter was cleared on abort (i.e. starts with 0 again) */
    *actual = -1;
    pa_when_abort (value == 42, Counter, (unsigned*)actual);
    *actual = -2;
    pa_pause;
    
    /* Test condition immediately true. */
    *actual = -1;
    pa_when_abort (value == 42, Counter, (unsigned*)actual);
    *actual = -2;
    pa_pause;
    
    /* Test completion when condition will not become true. */
    *actual = -1;
    pa_when_abort(value == 42, Delay, 2);
    *actual = -2;
} pa_end;

pa_activity (TestWhenAbortCheck, pa_ctx(), int actual, int expected) {
    pa_always {
        assert(actual == expected);
    } pa_always_end;
} pa_end;

pa_activity (TestWhenAbort, pa_ctx(pa_co_res(4); int value; int actual; int expected;
                                   pa_use(TestWhenAbortSpec); pa_use(TestWhenAbortTest); pa_use(TestWhenAbortCheck))) {
    pa_co(3) {
        pa_with_weak (TestWhenAbortSpec, &pa_self.value, &pa_self.expected);
        pa_with (TestWhenAbortTest, pa_self.value, &pa_self.actual);
        pa_with_weak (TestWhenAbortCheck, pa_self.actual, pa_self.expected);
    } pa_co_end;
} pa_end;

/* When-Reset Tests */

pa_activity (TestWhenResetSpec, pa_ctx(), int* value, int* expected) {

    /* Test no reset when done. */
    *expected = 2;
    pa_pause;
    *expected = 1;
    pa_pause;
    *expected = 0;
    pa_pause;
    *expected = -1;
    pa_pause;
    
    /* Test reset while not done. */
    *expected = 3;
    pa_pause;
    *expected = 2;
    pa_pause;
    *value = 42;
    *expected = 3;
    pa_pause;
    *value = 0;
    *expected = 2;
    pa_pause;
    *expected = 1;
    pa_pause;
    *expected = 0;
    pa_pause;
    *expected = -1;
} pa_end;

pa_activity (TestWhenResetTest, pa_ctx(pa_co_res(2); pa_use(CountDown)), int value, int* actual) {
    /* Test no reset when done. */
    *actual = -1;
    pa_when_reset (value == 42, CountDown, 3, (unsigned*)actual);
    *actual = -1;
    pa_pause;

    /* Test reset while not done. */
    *actual = -1;
    pa_when_reset (value == 42, CountDown, 4, (unsigned*)actual);
    *actual = -1;
} pa_end;

pa_activity (TestWhenResetCheck, pa_ctx(), int actual, int expected) {
    pa_always {
        assert(actual == expected);
    } pa_always_end;
} pa_end;

pa_activity (TestWhenReset, pa_ctx(pa_co_res(4); int value; int actual; int expected;
                                   pa_use(TestWhenResetSpec); pa_use(TestWhenResetTest); pa_use(TestWhenResetCheck))) {
    pa_co(3) {
        pa_with_weak (TestWhenResetSpec, &pa_self.value, &pa_self.expected);
        pa_with (TestWhenResetTest, pa_self.value, &pa_self.actual);
        pa_with_weak (TestWhenResetCheck, pa_self.actual, pa_self.expected);
    } pa_co_end;
} pa_end;

/* Test Driver */

#define run_test(nm) \
    pa_use(nm); \
    pa_init(nm); \
    while (pa_tick(nm) == PA_RC_WAIT) {}

int main(int argc, char* argv[]) {
    printf("Start\n");

    run_test(TestAwait);
    run_test(TestRun);
    run_test(TestCo);
    run_test(TestWhenAbort);
    run_test(TestWhenReset);

    printf("Done\n");

    return 0;
}
