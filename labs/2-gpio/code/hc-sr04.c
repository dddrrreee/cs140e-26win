/*
 * Functions used for the hc-sr04 extension
 */
#include "rpi.h"


#define CLOCK_SPEED_US 700 // cycles/us (clock speed)
#define SPEED_SOUND 343 // m/s (speed of sound)

#define HC_SR04_TIMEOUT 42000 // 60us to clock cycles

// Returns in micrometers. Stop measuring at 60us
unsigned hc_sr04_meas(unsigned trig, unsigned echo) {
    // Send 10us pulse

    // When 0, count how many cycles until 1, or timeout

    volatile unsigned cycle = cycle_cnt_read();

    while (!gpio_read(echo)) {} // deadlocks until it goes high lmao
    volatile unsigned cycle = cycle_cnt_read();
    
    while(gpio_read(echo)) {

        if (cycle_cnt_read() - cycle > HC_SR04_TIMEOUT)
            return ~0; // Timed out
    }

    volatile unsigned time_us = (cycle_cnt_read() - cycle) / CLOCK_SPEED_US;

    return time_us * SPEED_SOUND;



}