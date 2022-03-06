/* demo.c */

/* Includes */

#include "proto_activities.h"

#include <stdio.h>
#include <unistd.h>

/* LED */

enum Color {
    RED, BLACK
};

void setLED(enum Color color) {
    switch (color) {
        case RED: printf("LED = red\n"); break;
        case BLACK: printf("LED = black\n"); break;
    }
}

/* Activitis */

/* This allows to delay for a multiple of the base tick. */
pa_activity (Delay, pa_ctx(unsigned i), unsigned ticks) {
    pa_self.i = ticks;
    while (pa_self.i > 0) {
        --pa_self.i;
        pa_pause;
    }
} pa_activity_end;

/* This blinks an LED with 2 ticks red and 1 tick off(black). */
pa_activity (Blink, pa_ctx(pa_use(Delay))) {
    while (true) {
        setLED(RED);
        pa_run (Delay, 2);

        setLED(BLACK);
        pa_run (Delay, 1);
    }
} pa_activity_end;

/* This drives a blinking LED and preempts it after 10 ticks. */
pa_activity (Main, pa_ctx(pa_co_res(2); pa_use(Blink); pa_use(Delay))) {
    printf("Begin\n");
    
    pa_co(2) {
        pa_with (Delay, 10);
        pa_with_weak (Blink);
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

