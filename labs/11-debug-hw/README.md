## Eratta

Ignore `0-bit-ops`.

## Using debug hardware to catch mistakes

<p align="center">
  <img src="images/fetch-quest-task.png" width="450" />
</p>

Last lab you used single-stepping.  This lab you will build it, as 
well as watchpoints, which are similarly useful (we do fun tricks
with them in 240lx).

While last lab was fairly conceptual, today's will mostly be a
"fetch-quest" focused on defining inline assembly routines and calling
them.  Should be fairly mechanical, straightforward but useful lab.

Reminder:
  - Do the readings in the [PRELAB.md](./PRELAB.md) first!    

All the readings you need:
  - [Debug hardware chapter 13 for our arm1176 chip](./docs/arm1176-ch13-debug.pdf)

  - [Debug hardware cheat sheet](./../../notes/debug-hw/DEBUG-cheat-sheet.md)
    summarizes a the page numbers and rules (don't sleep on these).
  - [Fault registers](./docs/arm1176-fault-regs.pdf).

As you recall from last lab: The ARM chip we use, like many machines,
has a way to set both *breakpoints*, which cause a fault when the progam
counter is set to a specified address, and *watchpoints* which cause
a fault when a load or store is performed for a specified address.
These are usually used by debuggers to run a debugged program at
full-speed until a specific location is read, written or executed.

In addition to the single-stepping that you already saw, this lab will use
watchpoints as a way to detect memory corruption efficiently.  As you know
too-well by now, challenge of bare-metal programming is that we have not
had protection against memory corruption.  By this point in the quarter,
I believe everyone in the class has had to waste a bunch of time figuring
out what was causing memory corruption of some location in their program.
After today's lab you should be able to detect such corruption quickly:
   1. Simply set a watchpoint on the address of the memory getting corrupted,
   2. When a load or store to this address occurs, you'll immediately
      get an exception, at which point you can print out the program
      counter value causing the problem (or if you're fancy, a backtrace)
      along with any other information you would find useful.
Yes, we can (and will) do memory protection with virtual memory, but that
also requires a lot of machinery and can easily make real-time guarantees
tricky.  ARM virtual memory also only provides page-level protection,
whereas the watchpoints can track a single word (or even byte).

The lab today should be around 100-200 lines of code, doesn't need
virtual memory, and if you continue to do OS or embedded stuff, will be
very useful in the future.

### Checkoff
   - The tests for 1 and 2 pass.
   - Last lab uses your code.
   - You code isn't ugly and has comments where you got the instructions from.
   - There's a bunch of fun extensions.

### The best extension: match-breakpoint replay.

One of the most interesting extensions (favorite from last year):
  1. Run your code in single-step (breakpoint mismatch) mode, recording
     the trace of all registers in a buffer.
  2. Then replay the same code using breakpoint matching, checking that
     the results are identical to those in your trace buffer.
     This is a very harsh test that is near-verification that your
     matching code works (along with all our context switching).
  3. Then, exploit the fact that matching works at privileged-level
     and replay again at supervisor level, checking that the code behaves
     identically to the user-mode (other than the mode bits will be
     different).  This is a great way to detect non-virtualizable 
     instructions that silently behave differently at user vs 
     privileged mode.

This is super fun, not much code, and has some nice puzzles.  You'll
probably go your entire career never meeting anyone else who has done
this hack (though, perhaps, for good reason :).

----------------------------------------------------------------------
### Background

The arm1176jzf chip we are using has multiple "debug" modes.  We are
using "monitor debug-mode" which just means that you can configure the
hardware to throw exceptions when the contents of a small set of addresses
are read, written or executed.  The manual confusingly refers to the
addresses as *modified virtual addresses* (MVAs), but as far as I can
tell, you can also use physical addresses.   (I can't find any sentence
that gives us any guarantee of this, so if you see one let me know!).

As we've seen repeatedly: 
  - The ARM will control functionality through different co-processors.  For
    debuging functionality, that is co-processor 14.
  - There will typically be a configuration register that controls whether
    the functionality is enabled or disabled.  For us, this is the
    "debug status and control register" (`DSCR`) on 13-7.

The list of the hardware debug registers is on page 13-5.

A cheat-sheet of assembly instructions to access these registers (13-26):
<p align="center">
  <img src="images/cp14-asm.png" width="450" />
</p>

When a debug fault happens, the hardware will put additional values in
different fault registers.  A short cheat sheet of the assembly code to
get various of the fault registers:
<p align="center">
  <img src="images/cheat-sheet-fault-regs.png" width="450" />
</p>

-----------------------------------------------------------------------------
### Part 0: The pieces you need for this lab.

Much of this lab is using privileged instructions to read and write
debug registers, typically after modifying a few bits within it.
So you'll need to be clear on:
   1. How to emit inline assembly routines.
   2. How to manipulate sub-word bit-ranges 

We have examples for both.

##### Inline assembly

A good way to get started is to see how to define inline assembly to 
access the debug ID register(`DIDR`).  For this:
  - Review the end of the 
    [debug hardware cheat sheet](./../../notes/debug-hw/DEBUG-cheat-sheet.md)
    where it discusses how to emit assembly.

You can do this by hand.  But to make it easier,
`libpi/include/asm-helpers.h` defines useful macros that use C proprocessor
tricks to generate routines that use inline assembly to get and set
co-processor registers.
  - `cp_asm` generates both get and set methods.
  - `cp_asm_get` generates just a get method.
  - `cp_asm_set` generates just a get method.

As an example:
```
  cp_asm_get(cp14_didr, p14, 0, c0, c0, 0)
```
Generates the routine:

```
   static inline uint32_t cp14_didr_get(void) { 
        uint32_t ret=0; 
        asm volatile ("mrc p14, 0, %0, c0, c0, 0" : "=r" (ret)); 
        return ret; 
    }
```
That returns the DIDR register.  

Makes things easier.  

If you don't like our macro, write your own cleaner
version for an extension.  The CPP preprocessor is limited, but you can
abuse it in various ways to do useful things albeit in an ugly way.
The Rust (and Zig?) hackers here should be able to do some clever clean
things.

##### Bit-operations: `libc/bit-support.h`

The debug registers in cp14 are packed with different bit-sized
quantities.  There's a couple options for dealing with these:

  1. use bit-manipulation functions (`0-example-bitops.c` has examples)
  2. use structures that have fields (`0-example-debug-id.c` shows one)
  3. use raw shifts and masks.  This tends to make your code harder to follow.

For today we'll just do bit-manipulation (option 1).  While you You
can certainly write your own bit manipulation routines, it's easy to
make a mistake.  There's a bunch of routines in `libc/bit-support.h`
that you can use.

The example program `code/0-example-debug-id.c` shows some calls.
You can modify it to test different things out.
```
    // 13-6: get the debug id register value
    uint32_t didr = cp14_didr_get();

    // sanity check it using the values given on 13-7.
    uint32_t
    wrp     = bits_get(didr, 28, 31),
    brp     = bits_get(didr, 24, 27),
    version = bits_get(didr, 16, 19),
    variant = bits_get(didr, 4, 7),
    rev     = bits_get(didr, 0, 4);
```

-----------------------------------------------------------------------------
### Part 1: build a simple watchpoint library: `watchpoint.c`

***BEFORE YOU START***:
  - Make sure `make check` works.  It uses staff code and should
    pass the tests.
  - You will implement the code in `watchpoint.c`.

Tests:
  - `1-watchpt-test.c` --- set a single watchpoint.
  - `1-watchpt-byte-test.c` --- set a single watchpoint and make
     sure you trap sub-word accesses.

So far this quarter we've been vulnerable to load and stores of NULL
(address 0).  For example, if you run this code it will execute
"successfully"!

    #include "rpi.h"
    void notmain(void) {
        *(volatile unsigned *)0 = 0x12345678;
        printk("NULL = %x\n", *(volatile unsigned *)0);
    }

(Also: as we've seen in the original interrupt lab: we deliberately copied
exception handlers to address 0.)

As mentioned above and in the readings, the ARM chip we're using provides
*watchpoints* to trap when an address is used in a load or store and
*breakpoints* for when you try to execute the contents of an address.
The exception you receive for each is different.

For both, there will be one register that you put the address to watch in,
and a second, paired register to control what happens when the address
is used.

Note:
  - You want to look at the recipe for "how to set a simple watchpoint"
    in the debug chapter.
  - The test case for this part --- `1-watchpt-test.c` --- has some
    references to the different parts of the document you need.

With that said, we inline some of the key facts below.

To initialize co-processor 14:
  - We need to install exception handlers (do not enable interrupts):
    the code does this for you, using the same method as last lab.
  - You'll then need to enable any bits in the status register.

To set a watchpoint you can follow the recipe on 13-47.
  1. Enable monitor debugging using the `DSCR` (13-7): bits 14 and 15.
  2. Set the "watchpoint value register" (WVR) on 13-20 to 0.
  3. Set the "watchpoint control register" (WCR) on 13-21.
  4. After finishing your modifications of cp14, make sure you do a
     `prefetch_flush` (see below) to make sure the processor refetches
     and re-decodes the instructions in the instuction prefetch buffer.
  5. Implement the code in the data abort handler
     to check if the exception is from a debug exception and, if not
     crash with an error, otherwise handle it.  (The test already
     does this.)

For the WCR: We don't want linking (which refers to the use of context id's).
We do want:
   - Watchpoints both in secure and non-secure;
   - For both loads and stores.
   - Both priviledged and user.
   - Enabled.
   - Byte address select for all accesses (0x0, 0x1, 0x2, 0x3).

When you are done, both tests should pass and print `SUCCESS`.

After any modification to a co-processor 14 register, you have to do a 
`PrefetchFLush`:
<p align="center">
  <img src="images/prefetch-flush.png" width="450" />
</p>



How to get the data fault status register (DFSR, page 3-64): 
<p align="center">
  <img src="images/dfsr-get.png" width="450" />
</p>

You can use the DFSR to get the cause of the fault from bits `0:3` if `bit[10]=0`:
<p align="center">
  <img src="images/data-fault-field.png" width="450" />
</p>

How to get the fault address register (FAR): 
<p align="center">
  <img src="images/far-get.png" width="450" />
</p>

-----------------------------------------------------------------------------
### Part 2: build a simple matching breakpoint library: `breakpoint.c`

What to do:
  - Implement the breakpoint matching code in `breakpoint.c`.
  - Make sure `2-match-test.c` passes.

Test `2-match-test.c` has a skeleton program to check that you can detect
a simple breakpoint.  It sets a breakpdoint on `foo` and repeatedly calls
it: the exception handler disables the breakpoint and returns.  It then
does repeated calls to `GET32` and `PUT32` making sure it can trap.

As above, differentiate that the exception was caused by a debug
exception.

How to get the instruction fault status register (IFSR): 
<p align="center">
  <img src="images/ifsr-get.png" width="450" />
</p>

-----------------------------------------------------------------------------
### Part 3: extend `breakpoint.o` to handle mismatch, run last lab.

You'll extend the `breakpoint.c` file to handle mismatch breakpoints.
Then make sure last lab runs.

Easiest approach:
  1. Copy the `0-crash-course` into a directory in this lab so you don't 
     break anything.
  2. Drop in your breakpoint code.
  3. Make sure the tests work the same.
  4. Delete our `staff-breakpoint.o` from the makefile.
  5. Celebrate.
  6. Then do the same for `1-interleave`.


-----------------------------------------------------------------------------
### Extension: port your watchpoint code to a simple interface

***NOTE: if you want to do this, let us know and we'll push the code.***

So far we've done very low level hacking to get things working --- this is
great for a time-limited situation, since there aren't much moving pieces.
It's not so great if you want to use the stuff later.

The final part of the lab is trivially wrapping your code up in a
simple-minded interface that (1) slightly abstracts the interface and
(2) handles determining exception type and just calling the required
client handler.

Here you'll wrap up your watchpoint code in a simple system that manages 
a single watchpoint:
  1. The interface is in the header file `mini-watch.h` which gives 
     the prototypes and the types.
  2. The implementation is in the file `mini-watch.c` --- you should be
     able to steal most of the code in `1-watchpt-test.c` for the
     different routines.

Two tests:
  - `3-mini-watch-test.c` just re-does `1-watchpt-test.c` in the new interface.
  - `3-mini-watch-byte-access.c` - checks that you fault on byte addresses.

-----------------------------------------------------------------------------
### Extension: port your breakpoint code to a simple single-step

***NOTE: if you want to do this, let us know and we'll push the code.***

Interface is in `mini-step.h`.  Your code should go in `mini-step.c`.
You call it with a routine and it will run it in single-step mode.
There are two tests :
  - `4-mini-step-trace-only.c`: traces the pc values that run.
  - `4-mini-step-diff.c`: prints the registers that changed when
    running (you can use this to infer instruction semantics).

So for the example code:

```
00008044 <nop_10>:
    8044:   e320f000    nop {0}
    8048:   e320f000    nop {0}
    804c:   e320f000    nop {0}
    8050:   e320f000    nop {0}
    8054:   e320f000    nop {0}
    8058:   e320f000    nop {0}
    805c:   e320f000    nop {0}
    8060:   e320f000    nop {0}
    8064:   e320f000    nop {0}
    8068:   e320f000    nop {0}
    806c:   e12fff1e    bx  lr
```

`4-mini-step-diff.c` will result in:

```
TRACE:notmain:about to run nop10()!
TRACE: cnt=0: pc=0x8044:  {first instruction}
TRACE: cnt=1: pc=0x8048:  {no changes}
TRACE: cnt=2: pc=0x804c:  {no changes}
TRACE: cnt=3: pc=0x8050:  {no changes}
TRACE: cnt=4: pc=0x8054:  {no changes}
TRACE: cnt=5: pc=0x8058:  {no changes}
TRACE: cnt=6: pc=0x805c:  {no changes}
TRACE: cnt=7: pc=0x8060:  {no changes}
TRACE: cnt=8: pc=0x8064:  {no changes}
TRACE: cnt=9: pc=0x8068:  {no changes}
TRACE: cnt=10: pc=0x806c:  {no changes}
TRACE:notmain:done nop10()!
```

-----------------------------------------------------------------------------
### Extension:  do the race condition checker from 240lx

A brusque writeup of 
[how to use single-stepping to do concurrency checking](https://github.com/dddrrreee/cs240lx-24spr/tree/main/labs/12-interleave-checker).

This is a very cool trick, also good for final projects.

-----------------------------------------------------------------------------
### Extension:  make a instruction profiler.

You've built a simple statistical sampling profiler: easy but misses
stuff.  Now you can make a profiler that doesn't miss anything.  Bulid
one, show that it works, use it to speed something up.

-----------------------------------------------------------------------------
### Extension: make an always-on assertion system

Assertions are great, but they  have the downside that they are only
checked when you call them.  If there is only one mutation location,
this can be easy: just put the assertion there.  Unfortunately if there
are many such sites or there is corruption,  by the time you check,
too much time has passed and you can't figure out what happened.

You can use watchpoints and breakpoints to make "always on" assertions.
  1. Set a store watchpoint on a memory location.
  2. Each store to this location will cause a fault.
  3. Disable the watchpoint.
  4. Run an assert check.
  5. If it fails, give an error.

How to continue?
  1. Set a breakpoint on the next instruction.
  2. Jump back and let the mutation occur.
  3. When you get the breakpoint exception, re-enable the watchpoint and
     disable the breakpoint.

-----------------------------------------------------------------------------
### Extension: handle multiple watch and breakpoints.

In general, as a safety measure, we should probably always enable
watchpoint and breakpoints on `NULL`.   However, we'd also like to be
able to catch breakpoints on other addresses.

Extend your code to add support for a second simultaneous watchpoint and
breakpoint to a different address.  In the handler differentiate it if
it is a null pointer or a from the second value.

For this:
  1. Set a breakpoint on `foo` and see that you catch it.
  2. Set a watchpoint on a value and see that you catch it.


-----------------------------------------------------------------------------
### Extension: a more general breakpoint setup.

We hard-coded the breakpoint numbers and watchpoints to keep things simple.
You'd probably want a less architecture-specific method.  One approach
is to allocate breakpoints/watchpoints until there are no more available.

    // returns 1 if there were free breakpoints and it could set.
    // 0 otherwise.
    int bkpt_set(uint32_t addr);
    // delete <addr>: error if wasn't already set.
    void bkpt_delete(uint32_t addr);

    // same: for watchpoints.
    int watchpt_set(uint32_t addr);
    int watchpt_delete(uint32_t addr);


Note: you may want to design your own interface. The above is likely
not the best one possible.

-----------------------------------------------------------------------------
### Extension:  Failure-oblivious computing.

Possibly the most-unsound paper in all of systems research is Rinard
et al's "Failure-oblivious computing", which made buggy server programs
"work" by handling memory corruption as follows:
   1. Discard out of bound writes.
   2. Return a random value for out of bound reads (starting at 0, 1, 2, ...).

We can only handle a single address, but we can do a similar thing.   Change
your exception code to take the complete set of registers, and restore from
this set (so that you can change it).

