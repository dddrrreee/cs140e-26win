### How to override routines in `.o` files at link-time

This directory shows an example of how to use the GNU linker `ld` to
interpose on all calls to a given routine (or set of routines) by using
the `--wrap` flag.


**What the example does:** Override the definition of `foo` in `trace-foo.c`

**How it works:**

  1. `Makefile` uses the `--wrap=foo` flag when linking
  2. The linker replaces all calls to `foo` with calls to `__wrap_foo`
  3. The linker renames the original definition of `foo` to `__real_foo`
  4. `trace-foo.c` defines `__wrap_foo`, which can do whatever it wants (logging, argument modification, etc.) before calling `__real_foo`

**Expected output when you run this on your pi:**

```
  % pi-install trace-foo.bin
  .. a bunch of stuff ...
  about to call foo(1,2)
  in wrap foo(1,2)
  calling real foo(2,3)
  real foo: have arguments (2,3)
  returning from wrap_foo
  returned  5
  DONE!!!
```

Note: The wrapper increments both arguments, so `foo(1,2)` becomes
`__real_foo(2,3)`, which returns 5 instead of 3.

The code is small, so look at it!

**Example: Overriding `GET32`**

We do the same steps as overriding `foo`:

  1. When linking the program, add a `--wrap=GET32` argument to the linker command
  2. The linker will replace all calls to `GET32` with calls to `__wrap_GET32`
  3. The linker will also rename the definition of `GET32` as `__real_GET32`

Then, to trace all `GET32` operations, you implement a
`__wrap_GET32(unsigned addr)` routine which:
  - Calls `__real_GET32(addr)` to get the value for `addr`
  - Prints both the address and the value returned
  - Returns the value (so the rest of the code works normally)

**You would override `PUT32` similarly.**

**For the lab:**

Implement `__wrap_GET32` and `__wrap_PUT32` to log all GPIO accesses.
Your wrapper sits transparently between `gpio.c` and the hardware.

-------------------------------------------------------------------
### Visual: How `--wrap=GET32` works

**BEFORE (normal linking):**

```
gpio.c:gpio_read() 
{   
    ... 
    val = GET32(0x20200034); ----> GET32:
    ...                        ldr r0,[r0]  @ load
}    ^                         bx lr        @ return
     |
     |
     +------------------ return 0x00000008
```


**AFTER (with `--wrap=GET32`):**

```
gpio.c:gpio_read()
{
  ...
  val = GET32(0x20200034); --> __wrap_GET32(0x20200034)
  ...                          {
}                              printk("GET32(%x) = %x\n", addr, val);
 ^                             val = __real_GET32(addr); ---> GET32:
 |                                                           ldr r0,[r0]
 |                                              <----------- bx lr
 |                                              return 0x00000008
 |                             return val;
 |                           }
 +------------------ return 0x00000008
```

**What you see on console:**
```
GET32(20200034) = 00000008
```
And the rest of the code runs the same as before!

**What the linker did:**
1. Redirected ALL calls: `GET32(...)` ==>  `__wrap_GET32(...)`
2. Renamed original: `GET32` ==> `__real_GET32`
3. Zero source changes to `gpio.c`!

-------------------------------------------------------------------
### Key insight

This technique lets you add tracing/debugging/validation to code without
changing the source. The wrapper sits transparently between the caller
and the callee. For cross-checking GPIO implementations, this means you
can trace all hardware accesses without modifying `gpio.c`.

