/* demo.c */

/* Includes */

#include "proto_activities.h"

#include <stdio.h>
#include <unistd.h>

/* LED */

enum Color {
    RED, BLACK
};

void setLED(int pin, enum Color color) {
    switch (color) {
        case RED: printf("LED[%d] = red\n", pin); break;
        case BLACK: printf("LED[%d] = black\n", pin); break;
    }
}

/* Activitis */

/* This blinks an LED on every other tick. */
pa_activity (FastBlinker, pa_ctx(), int pin) {
    while (true) {
        setLED(pin, RED);
        pa_pause;

        setLED(pin, BLACK);
        pa_pause;
    }
} pa_activity_end;

/* This blinks an LED on a custom schedule. */
pa_activity (SlowBlinker, pa_ctx_tm(), int pin, unsigned on_ticks, unsigned off_ticks) {
    while (true) {
        setLED(pin, RED);
        pa_delay (on_ticks);

        setLED(pin, BLACK);
        pa_delay (off_ticks);
    }
} pa_activity_end;

/* An activity which delays for a given number of ticks. */
pa_activity (Delay, pa_ctx_tm(), unsigned ticks) {
    pa_delay (ticks);
} pa_activity_end;


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
    } pa_co_end;
    
    printf("Done\n");
} pa_activity_end;

/* Driver */

pa_use(Main);

int main(int argc, char* argv[]) {
    
    /* Tick at 1Hz until done. */
    while (pa_tick(Main) == PA_RC_WAIT) {
        sleep(1);
    }
    
    return 0;
}

