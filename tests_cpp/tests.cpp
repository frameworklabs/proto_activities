// tests.cpp

// Includes

#include "proto_activities.h"

#include <iostream>
#include <assert.h>

// Gobals

__thread pa_time_t pa_current_time_ms;

// Helpers

namespace helpers {

pa_activity (Delay, pa_ctx(unsigned remaining), unsigned i) {
    pa_self.remaining = i;
    while (pa_self.remaining-- > 0) {
        pa_pause;
    }
} pa_end;

pa_activity (CountDown, pa_ctx(unsigned remaining), unsigned i, unsigned& value) {
    pa_self.remaining = i;
    while (pa_self.remaining-- > 0) {
        value = pa_self.remaining;
        pa_pause;
    }
} pa_end;

pa_activity (Counter, pa_ctx(unsigned value), unsigned& res) {
    pa_always {
        res = pa_self.value++;
    } pa_always_end;
} pa_end;

} // namespace helpers

// Tests

namespace tests {

// Await Tests

namespace await {

pa_activity (TestAwaitSpec, pa_ctx(), int& value, int& expected) {
    value = 0;
    expected = 1;
    pa_pause;
    pa_pause;
    value = 99;
    pa_pause;
    value = 42;
    expected = 2;
    pa_pause;
    pa_pause;
    value = 100;
    expected = 3;
} pa_end;

pa_activity (TestAwaitTest, pa_ctx(), int value, int& actual) {
    actual = 1;
    pa_pause;
    pa_await (value == 42);
    actual = 2;
    pa_await (value == 100);
    actual = 3;
} pa_end;

pa_activity (TestAwaitCheck, pa_ctx(), int actual, int expected) {
    pa_always {
        assert(actual == expected);
    } pa_always_end;
} pa_end;

pa_activity (TestAwait, pa_ctx(pa_co_res(4); int value; int actual; int expected;
                               pa_use(TestAwaitSpec); pa_use(TestAwaitTest); pa_use(TestAwaitCheck))) {
    pa_co(3) {
        pa_with_weak (TestAwaitSpec, pa_self.value, pa_self.expected);
        pa_with (TestAwaitTest, pa_self.value, pa_self.actual);
        pa_with_weak (TestAwaitCheck, pa_self.actual, pa_self.expected);
    } pa_co_end;
} pa_end;

} // namespace await

// Delay Tests

pa_activity (TestDelaySpec, pa_ctx(), int& value, int& expected) {
    expected = 1;
    pa_pause;
    expected = 2;
    pa_pause;
    pa_pause;
    expected = 3;
    pa_pause;
    expected = 5;
    pa_pause;

    pa_current_time_ms = 0;
    expected = 6;
    pa_pause;
    pa_current_time_ms = 9;
    pa_pause;
    pa_current_time_ms = 10;
    expected = 7;
    pa_pause;
    pa_current_time_ms = 11;
    expected = 9;
    pa_pause;

    expected = 10;
    pa_current_time_ms = -3;
    pa_pause;
    pa_current_time_ms = 0;
    pa_pause;
    expected = 11;
    pa_current_time_ms = 2;
    pa_pause;
} pa_end;

pa_activity (TestDelayTest, pa_ctx_tm(), int value, int& actual) {
    // tick based delay
    actual = 1;
    pa_delay (1);
    actual = 2;
    pa_delay (2);
    actual = 3;
    pa_pause;
    actual = 4;
    pa_delay (0);
    actual = 5;
    pa_pause;

    // time based dalay
    actual = 6;
    pa_delay_ms (10);
    actual = 7;
    pa_pause;
    actual = 8;
    pa_delay_ms (0);
    actual = 9;
    pa_pause;

    // test overflow
    actual = 10;
    pa_delay_ms(5);
    actual = 11;
    pa_pause;
} pa_end;

pa_activity (TestDelayCheck, pa_ctx(), int actual, int expected) {
    pa_always {
        assert(actual == expected);
    } pa_always_end;
} pa_end;

pa_activity (TestDelay, pa_ctx(pa_co_res(4); int value; int actual; int expected;
                               pa_use(TestDelaySpec); pa_use(TestDelayTest); pa_use(TestDelayCheck))) {
    pa_co(3) {
        pa_with_weak (TestDelaySpec, pa_self.value, pa_self.expected);
        pa_with (TestDelayTest, pa_self.value, pa_self.actual);
        pa_with_weak (TestDelayCheck, pa_self.actual, pa_self.expected);
    } pa_co_end;
} pa_end;

// Run Tests

pa_activity (TestRunSpec, pa_ctx(), int& value, int& expected) {
    expected = 1;
    pa_pause;
    expected = 0;
    pa_pause;
    expected = 42;
    pa_pause;

    // Test that activity is starting clean on second invocation.
    expected = 0;
    pa_pause;
    expected = 42;
    pa_pause;
    expected = 1;
} pa_end;

pa_activity (TestRunActivity, pa_ctx(int value), int& actual) {
    actual = pa_self.value;
    pa_pause;
    pa_self.value = 42;
    actual = pa_self.value;
    pa_pause;
} pa_end;

pa_activity (TestRunTest, pa_ctx(pa_use(TestRunActivity)), int value, int& actual) {
    actual = 1;
    pa_pause;
    pa_run (TestRunActivity, actual);
    
    // Test that activity is starting clean on second invocation.
    actual = 1;
    pa_run (TestRunActivity, actual);
    actual = 1;
} pa_end;

pa_activity (TestRunCheck, pa_ctx(), int actual, int expected) {
    pa_always {
        assert(actual == expected);
    } pa_always_end;
} pa_end;

pa_activity (TestRun, pa_ctx(pa_co_res(4); int value; int actual; int expected;
                               pa_use(TestRunSpec); pa_use(TestRunTest); pa_use(TestRunCheck))) {
    pa_co(3) {
        pa_with_weak (TestRunSpec, pa_self.value, pa_self.expected);
        pa_with (TestRunTest, pa_self.value, pa_self.actual);
        pa_with_weak (TestRunCheck, pa_self.actual, pa_self.expected);
    } pa_co_end;
} pa_end;

// Co Tests

pa_activity (TestCoSpec, pa_ctx(), int& value, int& expected) {
    
    // All strong
    expected = 1;
    pa_pause;
    pa_pause;
    pa_pause;
    expected = 2;

    // All weak
    pa_pause;
    expected = 3;

    // All but one weak
    pa_pause;
    pa_pause;
    expected = 4;
} pa_end;

pa_activity (TestCoTest, pa_ctx(pa_co_res(4);
                                pa_use_as_ns(helpers, Delay, Delay1); pa_use_as_ns(helpers, Delay, Delay2); pa_use_as_ns(helpers, Delay, Delay3)),
                         int value, int& actual) {
    // All strong
    actual = 1;
    pa_co(3) {
        pa_with_as (Delay, Delay1, 1);
        pa_with_as (Delay, Delay2, 3);
        pa_with_as (Delay, Delay3, 2);
    } pa_co_end;
    assert(!pa_did_abort(Delay1));
    assert(!pa_did_abort(Delay2));
    assert(!pa_did_abort(Delay3));
    actual = 2;

    // All weak
    pa_co(3) {
        pa_with_weak_as (Delay, Delay1, 1);
        pa_with_weak_as (Delay, Delay2, 3);
        pa_with_weak_as (Delay, Delay3, 2);
    } pa_co_end;
    assert(!pa_did_abort(Delay1));
    assert(pa_did_abort(Delay2));
    assert(pa_did_abort(Delay3));
    actual = 3;

    // All but one weak
    pa_co(3) {
        pa_with_weak_as (Delay, Delay1, 1);
        pa_with_weak_as (Delay, Delay2, 3);
        pa_with_as (Delay, Delay3, 2);
    } pa_co_end;
    assert(!pa_did_abort(Delay1));
    assert(pa_did_abort(Delay2));
    assert(!pa_did_abort(Delay3));
    actual = 4;
} pa_end;

pa_activity (TestCoCheck, pa_ctx(), int actual, int expected) {
    pa_always {
        assert(actual == expected);
    } pa_always_end;
} pa_end;

pa_activity (TestCo, pa_ctx(pa_co_res(4); int value; int actual; int expected;
                            pa_use(TestCoSpec); pa_use(TestCoTest); pa_use(TestCoCheck))) {
    pa_co(3) {
        pa_with_weak (TestCoSpec, pa_self.value, pa_self.expected);
        pa_with (TestCoTest, pa_self.value, pa_self.actual);
        pa_with_weak (TestCoCheck, pa_self.actual, pa_self.expected);
    } pa_co_end;
} pa_end;

// When-Abort Tests

pa_activity (TestWhenAbortSpec, pa_ctx(), int& value, int& expected) {

    // Test that preemption happens only when conditiopn is true.
    expected = 0;
    pa_pause;
    expected = 1;
    pa_pause;
    expected = 2;
    pa_pause;
    value = 42;
    expected = -2;
    pa_pause;

    // Test that Counter was cleared on abort (i.e. starts with 0 again)
    value = 0;
    expected = 0;
    pa_pause;
    value = 42;
    expected = -2;
    pa_pause;

    // Test condition immediately true.
    value = 42;
    expected = 0;
    pa_pause;
    expected = -2;
    pa_pause;
    
    // Test completion when condition will not become true.
    value = 0;
    expected = -1;
    pa_pause;
    pa_pause;
    expected = -2;
    pa_pause;

    // Test after abort.
    expected = 0;
    pa_pause;
    expected = -2;
    pa_pause;

    expected = 0;
    pa_pause;
    expected = 1;
    pa_pause;
    expected = -2;
    pa_pause;

    expected = -1;
    pa_pause;
    pa_pause;
    expected = -2;
    pa_pause;

    expected = -1;
    pa_pause;
    pa_pause;
    expected = -2;
    pa_pause;

    // Test after ms abort.
    pa_current_time_ms = 0;
    expected = 0;
    pa_pause;
    pa_current_time_ms = 9;
    expected = 1;
    pa_pause;
    pa_current_time_ms = 10;
    expected = -2;
    pa_pause;

} pa_end;

pa_activity (TestWhenAbortTest, pa_ctx_tm(pa_use_ns(helpers, Counter); pa_use_ns(helpers, Delay)), int value, int& actual) {
    
    // Test that preemption happens only when condition is true.
    actual = -1;
    pa_when_abort (value == 42, Counter, reinterpret_cast<unsigned&>(actual));
    assert(pa_did_abort(Counter));
    actual = -2;
    pa_pause;
    
    // Test that Counter was cleared on abort (i.e. starts with 0 again)
    actual = -1;
    pa_when_abort (value == 42, Counter, reinterpret_cast<unsigned&>(actual));
    assert(pa_did_abort(Counter));
    actual = -2;
    pa_pause;
    
    // Test condition immediately true.
    actual = -1;
    pa_when_abort (value == 42, Counter, reinterpret_cast<unsigned&>(actual));
    assert(pa_did_abort(Counter));
    actual = -2;
    pa_pause;
    
    // Test completion when condition will not become true.
    actual = -1;
    pa_when_abort (value == 42, Delay, 2);
    assert(!pa_did_abort(Delay));
    actual = -2;
    pa_pause;

    // Test after abort.
    actual = -1;
    pa_after_abort (1, Counter, reinterpret_cast<unsigned&>(actual));
    assert(pa_did_abort(Counter));
    actual = -2;
    pa_pause;

    actual = -1;
    pa_after_abort (2, Counter, reinterpret_cast<unsigned&>(actual));
    assert(pa_did_abort(Counter));
    actual = -2;
    pa_pause;

    actual = -1;
    pa_after_abort (2, Delay, 2);
    assert(pa_did_abort(Delay));
    actual = -2;
    pa_pause;

    actual = -1;
    pa_after_abort (3, Delay, 2);
    assert(!pa_did_abort(Delay));
    actual = -2;
    pa_pause;

    // Test after abort ms.
    actual = -1;
    pa_after_ms_abort (10, Counter, reinterpret_cast<unsigned&>(actual));
    assert(pa_did_abort(Counter));
    actual = -2;
    pa_pause;

} pa_end;

pa_activity (TestWhenAbortCheck, pa_ctx(), int actual, int expected) {
    pa_always {
        assert(actual == expected);
    } pa_always_end;
} pa_end;

pa_activity (TestWhenAbort, pa_ctx(pa_co_res(4); int value; int actual; int expected;
                                   pa_use(TestWhenAbortSpec); pa_use(TestWhenAbortTest); pa_use(TestWhenAbortCheck))) {
    pa_co(3) {
        pa_with_weak (TestWhenAbortSpec, pa_self.value, pa_self.expected);
        pa_with (TestWhenAbortTest, pa_self.value, pa_self.actual);
        pa_with_weak (TestWhenAbortCheck, pa_self.actual, pa_self.expected);
    } pa_co_end;
} pa_end;

// When-Reset Tests

pa_activity (TestWhenResetSpec, pa_ctx(), int& value, int& expected) {

    // Test no reset when done.
    expected = 2;
    pa_pause;
    expected = 1;
    pa_pause;
    expected = 0;
    pa_pause;
    expected = -1;
    pa_pause;
    
    // Test reset while not done.
    expected = 3;
    pa_pause;
    expected = 2;
    pa_pause;
    value = 42;
    expected = 3;
    pa_pause;
    value = 0;
    expected = 2;
    pa_pause;
    expected = 1;
    pa_pause;
    expected = 0;
    pa_pause;
    expected = -1;
} pa_end;

pa_activity (TestWhenResetTest, pa_ctx(pa_co_res(2); pa_use_ns(helpers, CountDown)), int value, int& actual) {
    
    // Test no reset when done.
    actual = -1;
    pa_when_reset (value == 42, CountDown, 3, reinterpret_cast<unsigned&>(actual));
    actual = -1;
    pa_pause;

    // Test reset while not done.
    actual = -1;
    pa_when_reset (value == 42, CountDown, 4, reinterpret_cast<unsigned&>(actual));
    actual = -1;
} pa_end;

pa_activity (TestWhenResetCheck, pa_ctx(), int actual, int expected) {
    pa_always {
        assert(actual == expected);
    } pa_always_end;
} pa_end;

pa_activity (TestWhenReset, pa_ctx(pa_co_res(4); int value; int actual; int expected;
                                   pa_use(TestWhenResetSpec); pa_use(TestWhenResetTest); pa_use(TestWhenResetCheck))) {
    pa_co(3) {
        pa_with_weak (TestWhenResetSpec, pa_self.value, pa_self.expected);
        pa_with (TestWhenResetTest, pa_self.value, pa_self.actual);
        pa_with_weak (TestWhenResetCheck, pa_self.actual, pa_self.expected);
    } pa_co_end;
} pa_end;

// Every Tests

pa_activity (TestEverySpec, pa_ctx(), int& value, int& expected) {
    
    // Test that we don't enter every on false condition.
    value = 0;
    expected = 0;
    pa_pause;
    pa_pause;
    pa_pause;
    value = 1;

    // Test that we always enter every on true condition.
    expected = 1;
    pa_pause;
    value = 0;
    pa_pause;
    pa_pause;
    value = 1;

    // Test that we enter every on alternating conditions
    expected = 1;
    pa_pause;
    value = 2;
    expected = 2;
    pa_pause;
    value = 3;
    expected = 2;
    pa_pause;
    value = 4;
    expected = 4;
    pa_pause;
    value = 5;
    expected = 10;
    pa_pause;
    
    // Test every_ms
    pa_current_time_ms = 0;
    value = 0;
    expected = 1;
    pa_pause;
    
    pa_current_time_ms = 1;
    pa_pause;
    
    pa_current_time_ms = 5;
    expected = 2;
    pa_pause;

    pa_current_time_ms = 9;
    pa_pause;

    value = 1;
    expected = 10;
    pa_pause;

    // Test whenever
    value = 0;
    expected = -1;
    pa_pause;
    pa_pause;
    value = 42;
    expected = 2;
    pa_pause;
    expected = 1;
    pa_pause;
    expected = 0;
    pa_pause;
    expected = 2;
    pa_pause;
    expected = 1;
    pa_pause;
    value = 0;
    expected = 1;
    pa_pause;

} pa_end;

pa_activity (TestEveryTestBody, pa_ctx(), bool cond, int value, int& actual) {
    pa_every (cond) {
        actual = value;
    } pa_every_end;
} pa_end;

pa_activity (TestEveryMsTestBody, pa_ctx_tm(), int& actual) {
    pa_every_ms (5) {
        ++actual;
    } pa_every_end;
} pa_end;

pa_activity (TestEveryTest, pa_ctx_tm(pa_use(TestEveryTestBody); pa_use(TestEveryMsTestBody); pa_use_ns(helpers, CountDown)), int value, int& actual) {

    // Test that we don't enter every on false condition.
    pa_when_abort (value == 1, TestEveryTestBody, false, 1, actual);

    // Test that we always enter every on true condition.
    pa_when_abort (value == 1, TestEveryTestBody, true, 1, actual);

    // Test that we enter every on alternating conditions
    pa_when_abort (value == 5, TestEveryTestBody, value % 2 == 0, value, actual);
    actual = 10;
    pa_pause;

    /* Test every_ms */
    actual = 0;
    pa_when_abort (value == 1, TestEveryMsTestBody, actual);
    actual = 10;
    pa_pause;

    // Test whenever
    actual = -1;
    pa_whenever (value == 42, CountDown, 3, reinterpret_cast<unsigned&>(actual));
    actual = -2;

} pa_end;

pa_activity (TestEveryCheck, pa_ctx(), int actual, int expected) {
    pa_always {
        assert(actual == expected);
    } pa_always_end;
} pa_end;

pa_activity (TestEvery, pa_ctx(pa_co_res(4); int value; int actual; int expected;
                               pa_use(TestEverySpec); pa_use(TestEveryTest); pa_use(TestEveryCheck))) {
    pa_co(3) {
        pa_with (TestEverySpec, pa_self.value, pa_self.expected);
        pa_with_weak (TestEveryTest, pa_self.value, pa_self.actual);
        pa_with_weak (TestEveryCheck, pa_self.actual, pa_self.expected);
    } pa_co_end;
} pa_end;

// Lifecycle Tests

namespace {
    bool did_defer = false;
    int suspend_count = 0;
    int resume_count = 0;
    int enter_count = 0;
    bool done = false;
}

pa_activity (TestLifecycleDeferAct, pa_ctx(pa_defer_res), bool await) {
    did_defer = false;
    pa_defer {
        did_defer = true;
    };
    pa_await (await);
} pa_end

pa_activity (TestLifecycleDeferActInRun, pa_ctx(pa_use(TestLifecycleDeferAct)), bool await) {
    pa_run (TestLifecycleDeferAct, await);
} pa_end

pa_activity (TestLifecycleDeferActInCo, pa_ctx(pa_co_res(2); pa_use(TestLifecycleDeferAct); pa_use_ns(helpers, Counter); unsigned dummy), bool await) {
    pa_co(2) {
        pa_with (TestLifecycleDeferAct, await);
        pa_with_weak (Counter, pa_self.dummy);
    } pa_co_end
} pa_end

pa_activity (TestLifecycleSusResAct, pa_ctx(pa_susres_res)) {
    pa_suspend {
        suspend_count++;
    };
    pa_resume {
        resume_count++;
    };
    pa_halt;
} pa_end

pa_activity (TestLifecycleNoSusResAct, pa_ctx()) {
    // empty
} pa_end

pa_activity (TestLifecycleSusResSpec, pa_ctx(), bool& suspend) {
    suspend = false;
    pa_pause;
    suspend = true;
    pa_pause;
    pa_pause;
    suspend = false;
    pa_pause;
} pa_end

pa_activity (TestLifecycleSusResBody, pa_ctx(pa_use(TestLifecycleSusResAct); pa_use(TestLifecycleNoSusResAct)), bool suspend) {
    pa_when_suspend (suspend, TestLifecycleSusResAct);
    pa_when_suspend (suspend, TestLifecycleNoSusResAct);
} pa_end

pa_activity (TestLifecycleEnterAct, pa_ctx(pa_enter_res)) {
    pa_enter {
        ++enter_count;
    };
    pa_pause;
    pa_pause;
} pa_end

pa_activity (TestLifecycleBody, pa_ctx_tm(pa_co_res(2); bool suspend = false;
                                          pa_use(TestLifecycleDeferAct);
                                          pa_use(TestLifecycleDeferActInRun);
                                          pa_use(TestLifecycleDeferActInCo);
                                          pa_use(TestLifecycleSusResSpec);
                                          pa_use(TestLifecycleSusResBody);
                                          pa_use(TestLifecycleEnterAct))) {
    pa_run (TestLifecycleDeferAct, true);
    pa_pause;
    pa_after_abort(2, TestLifecycleDeferAct, false);
    pa_pause;
    
    pa_run (TestLifecycleDeferActInRun, true);
    pa_pause;
    pa_after_abort(2, TestLifecycleDeferActInRun, false);
    pa_pause;

    pa_run (TestLifecycleDeferActInCo, true);
    pa_pause;
    pa_after_abort(2, TestLifecycleDeferActInCo, false);
    pa_pause;
    
    pa_co(2) {
        pa_with (TestLifecycleSusResSpec, pa_self.suspend);
        pa_with_weak (TestLifecycleSusResBody, pa_self.suspend);
    } pa_co_end

    pa_run (TestLifecycleEnterAct);

    done = true;
} pa_end

pa_activity (TestLifecycleTest, pa_ctx()) {
    assert(!did_defer);
    pa_pause;
    assert(did_defer);
    pa_pause;
    assert(!did_defer);
    pa_pause;
    assert(!did_defer);
    pa_pause;
    assert(did_defer);
    pa_pause;
    
    assert(!did_defer);
    pa_pause;
    assert(did_defer);
    pa_pause;
    assert(!did_defer);
    pa_pause;
    assert(!did_defer);
    pa_pause;
    assert(did_defer);
    pa_pause;
    
    assert(!did_defer);
    pa_pause;
    assert(did_defer);
    pa_pause;
    assert(!did_defer);
    pa_pause;
    assert(!did_defer);
    pa_pause;
    assert(did_defer);
    pa_pause;
    
    assert(suspend_count == 0);
    assert(resume_count == 0);
    pa_pause;
    assert(suspend_count == 1);
    assert(resume_count == 0);
    pa_pause;
    assert(suspend_count == 1);
    assert(resume_count == 0);
    pa_pause;
    assert(suspend_count == 1);
    assert(resume_count == 1);
    pa_pause;

    assert(enter_count == 1);
    pa_pause;
    assert(enter_count == 2);
    pa_pause;
    assert(enter_count == 3);
    pa_pause;

    assert(done);
} pa_end

pa_activity (TestLifecycle, pa_ctx(pa_co_res(2); pa_use(TestLifecycleBody); pa_use(TestLifecycleTest))) {
    pa_co(2) {
        pa_with (TestLifecycleBody);
        pa_with (TestLifecycleTest);
    } pa_co_end
} pa_end

// Signal Tests

pa_activity (TestSignalsBodySub, pa_ctx(), pa_signal& s1, pa_signal& s2) {
    assert(!s1); assert(!s2);
    pa_pause;

    assert(!s1); assert(!s2);
    pa_emit(s1); pa_emit(s2);
    assert(s1); assert(s2);
    pa_pause;

    assert(!s1); assert(!s2);
    pa_pause;
} pa_end

pa_activity (TestSignalsBody, pa_ctx(pa_enter_res; pa_use(TestSignalsBodySub);
                                     pa_def_signal(s1); pa_def_signal(s2))) {
    assert(!pa_self.s1); assert(!pa_self.s2);
    pa_pause;

    assert(!pa_self.s1); assert(!pa_self.s2);
    pa_emit(pa_self.s1); pa_emit(pa_self.s2);
    assert(pa_self.s1); assert(pa_self.s2);
    pa_pause;

    assert(!pa_self.s1); assert(!pa_self.s2);
    pa_pause;

    pa_run(TestSignalsBodySub, pa_self.s1, pa_self.s1);
} pa_end

pa_activity (TestSignals, pa_ctx_tm(pa_use(TestSignalsBody))) {
    pa_run (TestSignalsBody);
    pa_run (TestSignalsBody); // Test re-invocation
    pa_after_abort (2, TestSignalsBody);
    pa_run (TestSignalsBody); // Test re-invocation after abort
} pa_end

} // namespace tests

// Test Driver

#define run_test(ns, nm) \
    pa_use_ns(ns, nm); \
    pa_init(nm); \
    while (pa_tick(nm) == PA_RC_WAIT) {}

int main(int argc, char* argv[]) {
    std::cout << "Start" << std::endl;

    run_test(tests::await, TestAwait);
    run_test(tests, TestDelay);
    run_test(tests, TestRun);
    run_test(tests, TestCo);
    run_test(tests, TestWhenAbort);
    run_test(tests, TestWhenReset);
    run_test(tests, TestEvery);
    run_test(tests, TestLifecycle);
    run_test(tests, TestSignals);

    std::cout << "Done" << std::endl;

    return 0;
}
