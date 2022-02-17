# proto_activities

This uses the [protothreads](http://dunkels.com/adam/pt/) approach to enable activity oriented synchronous programming (as promoted by [Blech](https://blech-lang.org/)) in pure C.

## Example code

```
/// This allows to delay for a multiple of the base tick.
pa_activity (Delay, pa_ctx(unsigned i), unsigned ticks) {
  pa_self.i = ticks;
  while (pa_self.i > 0) {
    --pa_self.i;
    pa_pause;
  }
} pa_activity_end;

/// This blinks an LED with 2 ticks red and 1 tick black.
pa_activity (Blink, pa_ctx(pa_use(Delay))) {
  while (true) {
    setLED(RED);
    pa_run (Delay, 2);
    
    setLED(BLACK);
    pa_run (Delay, 1);
  }
} pa_activity_end;

/// This drives a blinking LED and preempts it after 10 ticks.
pa_activity (Main, pa_ctx(pa_co_res(2); pa_use(Blink); pa_use(Delay))) {
  pa_co(2) {
    pa_with (Delay, 10);
    pa_with_weak (Blink);
  } pa_co_end;
} pa_activity_end;
```
For a description of the statements, currently refer to the [Blech documentation](https://www.blech-lang.org/docs/user-manual/statements).

## Related projects

[Here](https://github.com/frameworklabs/ego) is a little robot with `proto_activities` running on three ESP32 nodes.
