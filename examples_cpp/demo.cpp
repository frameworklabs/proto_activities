// demo.cpp

#include "proto_activities.h"

#include <iostream>
#include <thread>
#include <chrono>

namespace common {

// An activity which delays for a given number of ticks.
pa_activity (Delay, pa_ctx_tm(), unsigned ticks) {
    pa_delay (ticks);
} pa_activity_end;

} // namespace common

namespace led {

enum class Color {
    RED, BLACK
};

void setLED(int pin, enum Color color) {
    switch (color) {
        case Color::RED: std::cout << "LED[" << pin << "] = red" << std::endl; break;
        case Color::BLACK: std::cout << "LED[" << pin << "] = black" << std::endl; break;
    }
}

} // namespace led

namespace blinker {

using namespace led;

// This blinks an LED on every other tick.
pa_activity (FastBlinker, pa_ctx(pa_defer_res), int pin) {
    pa_defer {
        setLED(pin, Color::BLACK);
    };
    while (true) {
        setLED(pin, Color::RED);
        pa_pause;

        setLED(pin, Color::BLACK);
        pa_pause;
    }
} pa_activity_end;

// This blinks an LED on a custom schedule.
pa_activity (SlowBlinker, pa_ctx_tm(pa_defer_res), int pin, unsigned on_ticks, unsigned off_ticks) {
    pa_defer {
        setLED(pin, Color::BLACK);
    };
    while (true) {
        setLED(pin, Color::RED);
        pa_delay (on_ticks);

        setLED(pin, Color::BLACK);
        pa_delay (off_ticks);
    }
} pa_activity_end;

} // namespace blinker

namespace driver {

// This drives blinking LEDs and preempts them after 3 and 10 ticks.
pa_activity (Main, pa_ctx_tm(pa_co_res(3); pa_use_ns(common, Delay); pa_use_ns(blinker, FastBlinker); pa_use_ns(blinker, SlowBlinker))) {
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

} // namespace driver

pa_use_ns(driver, Main);

int main(int argc, char* argv[]) {
    
    // Tick at 1Hz until done.
    while (pa_tick(Main) == PA_RC_WAIT) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}
