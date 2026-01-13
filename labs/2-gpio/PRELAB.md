# Prelab: Setup and GPIO

### Always obey the first rule of PI-CLUB

- **_IF YOUR PI GETS HOT TO THE TOUCH: UNPLUG IT_**
- **_IF YOUR PI GETS HOT TO THE TOUCH: UNPLUG IT_**
- **_IF YOUR PI GETS HOT TO THE TOUCH: UNPLUG IT_**
- **_IF YOUR PI GETS HOT TO THE TOUCH: UNPLUG IT_**
- **_IF YOUR PI GETS HOT TO THE TOUCH: UNPLUG IT_**

You likely have a short somewhere and in the worst-case can fry your laptop.

---

## TL;DR

This is broken down some more below, but please make sure:

1. You've done the reading (see below), especially the Broadcom reading.
2. You've done the prelab questions on Gradescope.
3. You've installed the `arm-none-eabi-gcc` toolchain (see
   [0-pi-setup](../0-pi-setup/README.md)).
4. You have a way to read/write a microSD card and connect a USB-A device to
   your computer.

There's more information below. Since this is the first hardware lab
and people have varying levels of experience, we figured it's better to
over-explain than under-explain; future labs will be more succinct.

## Reading

1. Read through the [GPIO](../../notes/devices/GPIO.md) and [device memory](./../../notes/devices/DEVICES.md) crash
   courses. You should have the [Broadcom
   document](../../docs/BCM2835-ARM-Peripherals.annot.PDF) open so you can go
   through the examples in the crash course
   (`../../docs/BCM2835-ARM-Peripherals.annot.PDF`).

2. After doing so, read through pages 4--7 and 91---96 of the broadcom
   document to see what memory addresses to read and write to get the GPIO pins
   to do stuff. The specific stuff we want to do: configure a pin as an output
   and turn it on and off, or configure it as an input and read its level.

   This is a low-level hardware document. It's okay if it's confusing! Just
   skim what you don't understand and try to pull out what you can. We will
   cover the necessary pieces in class.

3. Look through the `code` directory. You'll be implementing the routines in
   `gpio.c` which is used by three simple programs: `1-blink.c` `2-blink.c` and
   `3-input.c`. You only modify `gpio.c`. We provide prototypes (in `rpi.h`)
   and some trivial assembly routines in `start.S`.

4. Note: where the broadcom document uses addresses `0x7E20xxxx`, you'll use
   `0x2020xxxx`. The reason is complicated, but you can find a diagram
   explaining it on page 5 of the manual.

