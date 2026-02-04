#ifndef __TEST_INTERRUPTS_H__
#define __TEST_INTERRUPTS_H__
// aggregate all the test definitions and code.
#include "rpi.h"
#include "rpi-interrupts.h"
#include "rpi-inline-asm.h"
#include "cycle-count.h"

// global state [bad form: but hopefully makes lab code more obvious]
extern volatile int n_interrupt;
enum { out_pin = 21, in_pin = 20 };
enum { N = 1024*32 };

extern volatile int n_falling;
void falling_init(void);

void rising_init(void);
extern volatile int n_rising;

// compose rise and fall.
void rise_fall_init(void);

// setup timer interrupts.
void timer_test_init(void);

// compose rise and fall and timer.
void rise_fall_timer_init(void) ;

#endif
