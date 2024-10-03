// demo.cpp

// Includes

#include "proto_activities.h"

#include <iostream>
#include <thread>
#include <chrono>

// LED

enum Color {
    RED, BLACK
};

void setLED(int pin, enum Color color) {
    switch (color) {
        case RED: std::cout << "LED[" << pin << "] = red" << std::endl; break;
        case BLACK: std::cout << "LED[" << pin << "] = black" << std::endl; break;
    }
}

// Activities

// This blinks an LED on every other tick.
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
} pa_activity_end;

// This blinks an LED on a custom schedule.
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
} pa_activity_end;

// An activity which delays for a given number of ticks.
pa_activity (Delay, pa_ctx_tm(), unsigned ticks) {
    pa_delay (ticks);
} pa_activity_end;


// This drives blinking LEDs and preempts them after 3 and 10 ticks.
pa_activity (Main, pa_ctx_tm(pa_co_res(3); pa_use(Delay); pa_use(FastBlinker); pa_use(SlowBlinker))) {
    std::cout << "Begin" << std::endl;

    // Blink Fast LED for 3 ticks
    pa_after_abort (3, FastBlinker, 0);

    std::cout << "Aborted fast LED" << std::endl;

    // Blink both LED for 10 ticks
    pa_co(3) {
        pa_with (Delay, 10);
        pa_with_weak (FastBlinker, 0);
        pa_with_weak (SlowBlinker, 1, 3, 2);
    } pa_co_end;
    
    std::cout << "Done" << std::endl;
} pa_activity_end;

// Driver

pa_use(Main);

int main(int argc, char* argv[]) {
    
    // Tick at 1Hz until done.
    while (pa_tick(Main) == PA_RC_WAIT) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}
