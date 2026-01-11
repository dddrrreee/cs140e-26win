### Implement the routines in `gpio.c` to control the pi's GPIO pins.

Files:

- `gpio.c` the only place you will write code.
- `rpi.h` has the definitions of simple routines we provide to
  read/write device memory and the prototypes for the `gpio.c` routines.
- `start.S`: the trivial routines we provide (written in assembly).

Part 1:

- `1-blink.c` blinks an LED on pin 20.
- `2-blink.c` blinks two LEDs oppositely, one on 20, one on 21.
  Note: its easy to make a mistake in how you read the broadcom
  document!

Part 2:

- `3-loopback.c`: input test: use a jumper to write a value from one
   GPIO set to output (9) to another GPIO set to input (10) and display 
   it using the LEDs attached above.

- `4-act-blink.c`: blink the on-board rpi LED.  tests that you can
  write to a high GPIO.
