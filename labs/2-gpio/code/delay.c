#include "rpi.h"

// trivial delay routine: <nop()> is in start.S so it doesn't
// get optimized.
void delay_cycles(unsigned ticks) {
  while (ticks-- > 0)
    nop();
}

