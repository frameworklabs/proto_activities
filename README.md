# proto_activities

Uses the [protothreads](http://dunkels.com/adam/pt/) approach to enable imperative synchronous programming (as promoted by [Blech](https://blech-lang.org/)) in C or C++.

With this header-only library you can simplify your embedded programming projects by keeping a `delay` based approach but still enable multiple things to happen at once in a structured, modular and deterministic way.

## Example code

```C
/* This blinks an LED on every other tick. */
pa_activity (FastBlinker, pa_ctx(pa_defer_res), int pin) {
    pa_defer {
        setLED(pin, BLACK);
    };
    while (true) {
        setLED(pin, RED);
        pa_pause;

        setLED(pin, BLACK);
        pa_pause;
    }
} pa_end

/* This blinks an LED on a custom schedule. */
pa_activity (SlowBlinker, pa_ctx_tm(pa_defer_res), int pin, unsigned on_ticks, unsigned off_ticks) {
    pa_defer {
        setLED(pin, BLACK);
    };
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

In this example, a fast led is blinked for 3 ticks and then both the fast and a slow led are blinked concurrently for 10 ticks.

## Constructs

As can be seen in the example above, an activity is defined by the `pa_activity` macro which takes the
name of the activity as first parameter. 

This is followed by what is called a context (`pa_ctx(...)`) and which stores the 
state which should outlive a single tick. Also sub-activities used in the activity are declared here with the `pa_use(<SomeActivity>)` macro. Separate context elements need to be separated by a semicolon (`;`).
To use delays within an activity, use `pa_ctx_tm` instead of `pa_ctx`, which holds an implicit time variable.

After the context, place the input and output parameters of the activity.

At the end of an activity, use `pa_activity_end` or just `pa_end` to close it off. 

Within an activity you can place normal C control structures and the following synchronous statements.

For a detailed description of the statements, please currently refer to the [Blech documentation](https://www.blech-lang.org/docs/user-manual/statements) or look at the `proto_activities` test and example programs.

* `pa_pause`: will pause processing of an activity and resume it at the next tick
* `pa_halt`: will pause the activity forever
* `pa_await (cond)`: will pause the activity and resume it once `cond` becomes true
* `pa_await_immediate (cond)`: like `pa_await` but will *not* pause if `cond` is true in the current tick
* `pa_delay (ticks)`: will pause the activity for the given number of ticks
* `pa_delay_ms (ms)`: will pause the activity for the given number of milliseconds
* `pa_run (activity, ...)`: runs the given sub-activity until it returns
* `pa_return`: end an activity from within its body - otherwise returns implicitly at the end
* `pa_co(n)`: starts a concurrent section with `n` trails - reserve the number of trails with `pa_co_res(num_trails)` in the activities context - end section with `pa_co_end`
* `pa_with (activity, ...)`: runs the given activity concurrently with the others of this section - only applicable within `pa_co`
* `pa_with_weak (activity, ...)`: runs the given activity concurrently with the others of this section and can be preempted - only applicable within `pa_co`
* `pa_when_abort (cond, activity, ...)`: runs the given activity until `cond` becomes true in a subsequent tick - unless it ends before
* `pa_when_reset (cond, activity, ...)`: runs the given activity and restarts it when `cond` becomes true in a subsequent tick
* `pa_when_suspend (cond, activity, ...)`: will suspend the given activity while `cond` is true and lets it continue when `cond` is false again
* `pa_after_abort (ticks, activity, ...)`: will abort the given activity after the specified number of ticks
* `pa_after_ms_abort (ms, activity, ...)`: will abort the given activity after the specified time in milliseconds
* `pa_did_abort (activity)`: reports whether an activity was aborted in a call before
* `pa_always`: will run code on every tick - end block with `pa_always_end`
* `pa_every (cond)`: will run code everytime `cond` is true - end block with `pa_every_end`
* `pa_every_ms (ms)`: will run code now and every `ms` milliseconds thereafter - end block with `pa_every_end`. Note: Do *not* use any other construct which uses timing (like `pa_delay_ms`) in the enclosed block 
* `pa_whenever (cond, activity, ...)`: will run the given activity whenever `cond` is true and abort it if `cond` turns false

When compiling wit C++ you could also define the following lifecycle callbacks:

* `pa_defer`: defines an instantaneous block of code to run when the activity ends by itself or gets aborted. Add `pa_defer_res` annotation to the context to enable this feature.
* `pa_suspend`: defines an instantaneous block of code to run when an activity gets suspended by the surrounding `pa_when_suspend`. Add `pa_susres_res` annotation to the context to enable this feature.
* `pa_resume`: defines an instantaneous block of code to run when an activity gets resumed by the surrounding `pa_when_suspend`. Add `pa_susres_res` annotation to the context to enable this feature.
 

## Related projects

* A medium article about proto_activities can be found [here](https://medium.com/@zauberei02_ruhigste/boosting-embedded-real-time-productivity-with-imperative-synchronous-programming-22aa2eb38414).
* [Here](https://github.com/frameworklabs/ego) is a little robot with `proto_activities` running on three ESP32 nodes.
* See running proto_activities code in [this](https://wokwi.com/projects/385178429273730049) online Wokwi simulator. 
* [Blech](https://blech-lang.org/) is a new programming language for the embedded domain which inspired `proto_activities`.
* [Pappe](https://github.com/frameworklabs/Pappe) is a sibling project which uses an embedded DSL to allow Blech-style imperative synchronous programming in Swift.
