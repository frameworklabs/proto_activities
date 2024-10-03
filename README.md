# proto_activities

This uses the [protothreads](http://dunkels.com/adam/pt/) approach to enable imperative synchronous programming (as promoted by [Blech](https://blech-lang.org/)) in C or C++.

## Example code

```C
/* This blinks an LED on every other tick. */
pa_activity (FastBlinker, pa_ctx(), int pin) {
    while (true) {
        setLED(pin, RED);
        pa_pause;

        setLED(pin, BLACK);
        pa_pause;
    }
} pa_end

/* This blinks an LED on a custom schedule. */
pa_activity (SlowBlinker, pa_ctx_tm(), int pin, unsigned on_ticks, unsigned off_ticks) {
    while (true) {
        setLED(pin, RED);
        pa_delay (on_ticks);

        setLED(pin, BLACK);
        pa_delay (off_ticks);
    }
} pa_end

/* An activity which delays for a given number of ticks. */
pa_activity (Delay, pa_ctx_tm(), unsigned ticks) {
    pa_delay (ticks);
} pa_end


/* This drives blinking LEDs and preempts them after 3 and 10 ticks. */
pa_activity (Main, pa_ctx_tm(pa_co_res(3); pa_use(Delay); pa_use(FastBlinker); pa_use(SlowBlinker))) {
    printf("Begin\n");

    /* Blink Fast LED for 3 ticks */
    pa_after_abort (3, FastBlinker, 0);
    
    /* Blink both LED for 10 ticks */
    pa_co(3) {
        pa_with (Delay, 10);
        pa_with_weak (FastBlinker, 0);
        pa_with_weak (SlowBlinker, 1, 3, 2);
    } pa_co_end
    
    printf("Done\n");
} pa_end
```

As can be seen, an activity is defined by the `pa_activity` macro which takes the
name of the activity as first parameter. This is followed by what is called a context (`pa_ctx`) and which stores the 
state which should outlive a single tick. In this context, also the sub-activities used are declared with `pa_use`. Separate the context elements with a semicolon `;`.
If using delays, use the `pa_ctx_tm` instead, which holds an implicit time variable.
After the context, place the input and output parameters of the activity.
At the end of an activity, use `pa_activity_end` or just `pa_end` to close it off. 


For a detailed description of the statements, currently refer to the [Blech documentation](https://www.blech-lang.org/docs/user-manual/statements).

* `pa_pause`: will pause the activity for one tick
* `pa_halt`: will pause the activity forever
* `pa_await (cond)`: will pause the activity and resume it once `cond` becomes true
* `pa_await_immediate (cond)`: like `pa_await` but will pause if `cond` is true in the current tick. 
* `pa_delay (ticks)`: will pause the activity for the given number of ticks
* `pa_delay_ms (ms)`: will pause the activity for the giveb number of milliseconds
* `pa_run (activity, ...)`: runs the given sub-activity until it returns
* `pa_return`: end an activity from within its body - otherwise ends at the end
* `pa_co(n)`: starts a concurrent section - reserve the number of trails with `pa_co_res(num_trails)` in the context of an activity
* `pa_with (activity, ...)`: runs the given activity concurrently with the others of this section
* `pa_with_weak (activity, ...)`: runs the given activity concurrently with the others of this section and can be preempted
* `pa_when_abort (cond, activity, ...)`: runs the given activity until `cond` becomes true in a subsequent tick - unless it ends before
* `pa_when_reset (cond, activity, ...)`: runs the given activity and restarts it when `cond` becomes true in a subsequen tick
* `pa_when_suspend (cond, activity, ...)`: will suspend the given activity while `cond` is true and lets it continue when `cond` is false again
* `pa_after_abort (ticks, activity, ...)`: will abort the given activity after the specified number of ticks
* `pa_after_ms_abort (ms, activity, ...)`: will abort the given activity after the specified time in milliseconds
* `pa_did_abort (activity)`: reports whether an activity was aborted in the call before
* `pa_always`: will run code on every tick
* `pa_every (cond)`: will run code everytime `cond` is true
* `pa_whenever (cond, activity, ...)`: will run the given activity whenever `cond` is true and abort it if `cond` turns false

When compiling wit C++ you could also define the following lifecycle callbacks:

* `pa_defer`: defines an instantaneous block of code to run when the activity ends by itself or gets aborted
* `pa_suspend`: defines an instantaneous block of code to run when an activity gets suspended by the surrounding `pa_when_suspend`
* `pa_resume`: defines an instantaneous block of code to run when an activity gets resumed by the surrounding `pa_when_suspend`
 

## Related projects

* A medium article about proto_activities can be found [here](https://medium.com/@zauberei02_ruhigste/boosting-embedded-real-time-productivity-with-imperative-synchronous-programming-22aa2eb38414).
* [Here](https://github.com/frameworklabs/ego) is a little robot with `proto_activities` running on three ESP32 nodes.
* See running proto_activities code in [this](https://wokwi.com/projects/385178429273730049) online Wokwi simulator. 
* [Pappe](https://github.com/frameworklabs/Pappe) is a sibling project which uses an embedded DSL to allow Blech-style imperative synchronous programming in Swift.

