# Lab 2: write your own code to control GPIO; throw ours out.

<p align="center">
  <img src="images/rpi-mj.png" width="450" />
</p>



**Goal**: control the pins on the pi by writing your own bare-metal
GPIO driver using the raw Broadcom BCM2835 datasheet. A cornerstone
of this class is learning how to understand and act on datasheets.
This is our first "hello world" example: short, but representative.

**What you'll build**:
- `gpio_set_input(pin)/output(pin)` - Configure pins for input/output.
- `gpio_set_on(pin)/off(pin)` - Turn output pins on (3.3V) or off (0V).
- `gpio_read(pin)` - Read input pins: returns 0 or 1 based on
   input voltage level.
- Various LED blink programs using your driver.

**Prerequisites**:
- Read [PRELAB.md](PRELAB.md) before starting
- Complete [Lab 0: Pi Setup](../0-pi-setup/README.md)
- Slides (Lab cheat sheet): [slides/2-gpio-notes.pdf](./slides/2-gpio-notes.pdf)

**Required readings**:
- Broadcom BCM2835 manual: [../../docs/BCM2835-ARM-Peripherals.annot.PDF](../../docs/BCM2835-ARM-Peripherals.annot.PDF) - you cannot do the lab without this. Defines device addresses, values, and rules.
- [BCM2835 GPIO errata](https://elinux.org/BCM2835_datasheet_errata#p90) - Known BCM2835 bugs.
- [GPIO notes](../../notes/devices/GPIO.md) - How GPIO works.
- [DEVICE notes](../../notes/devices/DEVICES.md) - Common device hardware patterns.

**Read the code/ (10 minutes)**
- **Goal**: Understand there's no hidden magic in the system.
- All code for this lab is in `code/` (except bootloader, which you'll
  write in Lab 6). Look through:
  - `start.S` - boot assembly that sets up stack and calls C code
  - `Makefile` (heavily commented).  After you run make:
  - `.list` files - disassembly showing what actually runs
  - Memory layout: the code addresses come from `memmap`
     and correspond to the address in `config.txt`.

#### Checkoff

No autograder today: demo working tests to TA.  We will check: 
  - Does: `3-loopback.bin` work?
  - Does: your own `5-all.bin` work?
  - Does: your `gpio.c` make sense and exemplify simplicity?

Unfortunately, due to the size of the class this year we won't be able to give
you debugging help during checkoff if there are others waiting.

**Extensions**: See [the extensions doc](EXTENSIONS.md).

--------------------------------------------------------------------
## 0. Make sure your LED + pi hardware works.

**For eternity (and the rest of the quarter):** Always test hardware
before writing code. Otherwise you waste time not knowing if bugs are
in software, hardware, or (worse) both.

This isn't the first time you've deliberately solved for one variable
at a time!  Very old technique, so don't forget what you know because
you're in a new domain:
1. **As you learned in maths**: solving one variable at a time makes
   problems solvable at all, and ensures you solve for the right variable.
   - LOL: Debugging software when hardware is broken.
   - LOL: Debugging hardware when software is broken.
   - LOL: Debugging at all when both are broken.
2. **Break problems into pieces**: The smaller the piece, the easier
   to isolate its problem(s).
3. **Differential debugging**: Swap pieces to find not-working.
   Binary search is great, but even linear substitution beats
   solving two variables at once.
4. **Class pro-tip**: We always provide pre-built `staff-binaries/*.bin`
   to test hardware independently. Use them! (Want harder mode? Ignore these.)


#### Quick LED Test

Test your LED works by connecting directly to 3.3V (not 5V!) power, no code
needed:

1. Connect LED to ground and **briefly** touch other leg to 3.3V pin
  - **WARNING**: Again: Use 3.3V, NOT 5V!  (Written on board.)
  - EE folks: 
    - We don't use a breadboard b/c it's bulky.
    - We don't use resistors for the same reason + the 10mm LED is 
      big enough we (generally) don't fry it.
  - **SAFETY**: since these are farm-fresh LEDs from overseas don't
    put your eye right next to them in case they were made by maniacs
    and explode.
2. **LED doesn't light?**
   - Reverse the LED (longer leg = power, shorter = ground)
   - Try partner's working LED on your pi
   - Try your LED on partner's pi
   - Try different LED
   - If still broken → ask staff
3.  **Wire colors** (strongly recommended):
   - Red = power (3.3V)
   - Black = ground
   - Colors don't affect electricity, but make debugging much easier

**Power cycling**: For today only, you must power-cycle the pi (push the
metal Parthiv-board button or unplug the USB cable) after each test.
- Why: we don't have a reboot yet. If you look at `code/start.S`, which
  calls into our C code, you'll see it infinite loops on exit.
- If bootloader errors occur, this is usually why.
- (Fixed in later labs)

--------------------------------------------------------------------
## Part 1: make GPIO output work  (`1-blink.bin`, `2-blink.bin`)

#### Hardware Setup

1. Connect LED to pin 20 + ground. Both are labeled on the Parthiv-board.
2. Test: `pi-install staff-binaries/1-blink.bin` → LED should blink
3. Test: `pi-install staff-binaries/2-blink.bin` → LED and 
   Parthiv on-board LED should blink oppositely.

#### Implementation

Edit `code/gpio.c` to implement:

| Function | Purpose | Complexity |
|----------|---------|------------|
| `gpio_set_output(pin)` | Configure pin as output | ~10 lines |
| `gpio_set_on(pin)` | Turn pin on (set high) | ~5 lines |
| `gpio_set_off(pin)` | Turn pin off (set low) | ~5 lines |

**Datasheet reference**: BCM2835 manual, GPIO section (pages 89-101)

See [HINTS.md](HINTS.md) for implementation guidance.

#### Testing

After implementation, test both programs:

**Test 1 - Single LED**:
```bash
# Power-cycle pi (parthiv button)
pi-install code/1-blink.bin
# Pin 20 LED should blink
```

**Test 2 - Two LEDs**:
```bash
# Power-cycle pi
pi-install code/2-blink.bin
# Pins 20's led and 27 (parthiv led) should blink in opposite phase
```

<p float="left">
  <img src="images/part1-succ-green.jpg" width="250" />
  <img src="images/part1-succ-blue.jpg" width="250" />
</p>

#### Troubleshooting

**Tests fail but staff binary works?**
- Check jumper connections (may have come loose)
- Verify LED polarity
- Try staff binary again to isolate software vs hardware

**Both fail?**
- Hardware issue - recheck Part 0

See [HINTS.md](HINTS.md) for more debugging tips.

--------------------------------------------------------------------
### Part 2: GPIO Input (`3-loopback.bin`)

**New capability**: In part 1 you did output, now you'll do input.
Between the two you can control a surprising number of digital devices
you can buy on eBay, Adafruit, Sparkfun, Alibaba, etc.

**1. `gpio_set_input(pin)` - Configure pin as input**
- Few lines of code, similar to `gpio_set_output`
- **CRITICAL**: Use read-modify-write for `FSEL` register
  - Don't overwrite other pin configurations in the same FSEL register.
  - Bug may not appear today but will break multi-device programs later.

**2. `gpio_read(pin)` - Read pin value**
- Must return ONLY `0` or `1` (not arbitrary positive integers)
- Must mask out other pins' values
- Verify using bitwise operations

#### Hardware Setup - Loopback Test

We'll test input without external devices using a "loopback jumper" 
(common trick):
1. **SETUP**: Connect jumper from pin 9 (output) to pin 8 (input).
2. Code writes to pin 9, reads from pin 8.
3. Since we don't have `printf` yet, `3-loopback.bin` makes the system's
   state externally visible by turning LEDs on/off based on what pin
   8 reads.

```bash
# Power-cycle pi
pi-install code/3-loopback.bin
# Both LEDs should turn on/off simultaneously
```

<p float="left">
  <img src="images/part2-loop-on.jpg" width="250" />
  <img src="images/part2-loop-off.jpg" width="250" />
</p>

--------------------------------------------------------------------
## Part 3: write your own tests.

#### write `4-act-blink.c`

The rpi has an on-board LED controlled by GPIO pin 47.  It is "active low"
in that when you set pin(47)=1, it turns off, when pin(47)=0, it turns
on.  The ACT led is useful as a status indicator since it's built-in.
It's also useful as a check that you can access higher GPIO bank.  

Note: it's very small, and right below the Parthiv LED:
<p float="center">
  <img src="images/part3-act-on.jpg" width="250" />
</p>

**What to do**:
 1. Create a new file `4-act-blink.c`
 2. Write a program to that blinks the ACT led the same as `1-blink.c`
 3. Modify the Makefile so it compiles. 
 4. Compile and run it!


#### write `5-all.c`

As the final test:
 1. Write another test `5-all.c` that blinks all three LEDs
    at the same time in unison (all on, all off).  This makes it easy
    to see that everything is working.
 2. Modify the `Makefile`, compile, run as above.

Congratulations!  At this point you can check-off.

--------------------------------------------------------------------
## Extension: print "hello world" using your GPIO + time.

If you finish early, and get tired of helping other people, a cool
extension is to use your GPIO code to print "hello world" from your pi
to your laptop "bit-banging" (manually setting pins high and low at a
semi-precise rate).

The Unix-side bootloader `pi-install` sends and receives bytes from the
pi using a 8n1 UART protocol configured at 115,200 baud (115,200 bits
per second).  It prints any bytes it receives from the pi.  So if you
can bit-bang "hello world" correctly, it will print it (and anything
else you send).

The [wikipedia entry for the UART protocol](https://en.wikipedia.org/wiki/Universal_asynchronous_receiver-transmitter) has a reasonable description of how
transmit a byte `x` at a baud rate B.

For a given baud rate compute how many micro-seconds `T` you write each bit.  
 - For example, for 115,200, this is: `(1000*1000)/115200 = 8.68`.  
 - NOTE: you can use either cycles or micro-seconds.  The former is
   much easier make accurate.
 - The A+ runs at `700MHz` so that is 700 * 1000 * 1000 cycles per
   second or about `6076` cycles per bit.)

To initialize:
  1. Take over the hardware UART TX pin (GPIO 14 )by setting it as ouput .
  2. Make sure the TX line has its correct default value.

To transmit:
  1. write a 0 (start) for T.
  2. write each bit value in the given byte for T (starting at bit 0,
     bit 1, ...).
  3. write a 1 (stop) for at-least T.

Cycle counter inline assembly if you need it:

```c
// from ch3 of the arm1176.pdf manual in doc/

// must do init first.
static inline void cycle_cnt_init(void) {
    uint32_t in = 1;
    asm volatile("MCR p15, 0, %0, c15, c12, 0" :: "r"(in));
}

// read cycle counter: should add a write().
static inline uint32_t cycle_cnt_read(void) {
    uint32_t out;
    asm volatile("MRC p15, 0, %0, c15, c12, 1" : "=r"(out));
    return out;
}
```

Given this, you can then:
  1. Make a `putk` that prints arbitrary strings.
  2. Then start making your own `printk` that prints integers (`%d`)
     and other types.
  3. At that point you have enough to make your own logic analyzer,
     which can be fun.  (See 340lx's first lab from 2025 :). 


There's a bunch of other extensions in [the extensions doc](EXTENSIONS.md).

--------------------------------------------------------------------
## Additional information

More links:

1. Useful baremetal information: (http://www.raspberrypi.org/forums/viewtopic.php?t=16851)

2. More baremetalpi: (https://github.com/brianwiddas/pi-baremetal)

3. And even more bare metal pi: (http://www.valvers.com/embedded-linux/raspberry-pi/step01-bare-metal-programming-in-cpt1)

4. Finally: it's worth running through all of dwelch's examples:
   (https://github.com/dwelch67/raspberrypi).

<p align="center">
  <img src="images/led-breaker.jpg" width="450" />
</p>


--------------------------------------------------------------------
## Some yapping

Bigger picture: it's easy to not pay attention and view these GPIO pieces
we built today as baby-stuff, only interesting for script-kiddies and
Arduino forums.

But, with the addition of accurate time (e.g., using a cycle counter),
you have enough to make you own implementations of nearly arbitrary
digital protocols, including the ones the pi has hardware for: SPI,
UART, I2C (with some additions), or more general ones such as ws2812b
addressable light arrays (useful if you want a free burning man ticket)
or your own hyper-fast logic analyzer that can beat the speed of many
commercial ones (at least circa a few years ago).

A favorite thing about math classes back when I was a kid was that
you'd have two pages of abstract algebra and then 10 pages of absolutely
insane problems proving weird stuff about Abelian groups that, in fact,
the 2 pages gave you enough machinery for.  So little starter code!
So many tricks!

This class is not at that level (though our TAs are), but this is one of
those examples.  A big unlock as a software person is to realize that
often all the complicated stuff going on in a piece of hardware or a
hardware protocol just reduces down to combining 1 + 0 + time.

If you think about this for a bit, you'll have much more power than a few
hours ago.
