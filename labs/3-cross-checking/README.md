### Lab: grade your own GPIO code by cross-checking

**Goal:** Verify your `gpio.c` from Lab 2 is correct by comparing its
behavior against everyone else's. 

**Method:** Trace all `PUT32`/`GET32` calls, compute checksums (collision
resistant hash), compare.

**Key Insight:** Hard to prove code is correct absolutely. Easy to prove
it's equivalent to someone else's. If even one person is correct and
your code matches theirs then you're correct too.

**What you'll build:**

1. Run your `gpio.c` in a fake-pi simulator on Unix (no hardware needed)
  - Automated equivalence checking via checksums
2. Hardware tracing using linker `--wrap` trick
  - Automated equivalence checking via checksums
3. Write more GPIO routines that get checked by (1) and (2).

**Artifacts you'll have at the end:**

- A Unix-runnable version of your pi code (huge for debugging)
- Guarantees your GPIO code actually works
- A technique you'll use variants of for the rest of the quarter

### Why we do this lab

**Goal:** You write every line of (interesting) low-level code in
CS140E. No libraries, no magic. Good: you understand everything. Bad:
one bug in your code can make your quarter a miserable experience.
After today's lab you'll be surprised if your code is broken (we will
be too!).

**Why this matters:**

- **Hardware-level bugs are often Heisenbugs, not Bohr bugs.** They don't
crash deterministically in the same place each time ("Bohr bugs"). Instead
they appear non-deterministically with odd effects ("Heisenbugs"). If
you can't replicate a bug, good luck fixing it.

- **This problem is not theoretical.** We short-cut this lab once (2022)
with sloppy cross-checking. Result: constant stream of people with
"weird" multi-hour bugs all quarter. One person finally got hit by a
Lab 2 bug in Lab 14 (networking). Took several days to find because:
(1) the bug was non-deterministic, and (2) he was focused on Lab 14 code
— why would you check Lab 2? It had been "working" all quarter!

- **After we went back to stringent checking:** No `gpio.c`
problems. After today's lab, as long as you don't modify your GPIO code
without re-checksumming, you won't hit `gpio.c` bugs. If you do: tell us!
After cross-checking it is *interesting* if you still hit a bug, unlike
normal methods such as   testing or code inspecting, where
hitting (yet another) bug surprises no one.

**The Core Idea: Cross-Checking via Equivalence**

Two approaches to correctness:

**Hard way (constructive):** "Is my code correct?"
- Need complete specification
- Handle all corner cases
- Account for hardware errata
- Write complex test harness
- Still misses bugs

**Easy way (differential):** "Is my code equivalent to yours?"
- Run both on same inputs
- Compare outputs
- Different outputs → someone is wrong
- Same outputs → probably both correct
- If one person is correct, everyone who matches is correct

**Concrete example:**

Your `gpio.c` might look completely different from your partner's:
- Different variable names
- Different code structure
- Different control flow

But if you both call:
```
PUT32(0x20200008, 0b001);  // Set pin=20 output
PUT32(0x2020001C, 1<<20);  // Turn pin 20 on
```
in the same order → functionally equivalent.

**The Turing Machine Analogy:**

Hard to analyze two Turing machines by comparing their transition tables.

Easy to check equivalence: run them on the same input, compare tapes. Same tape → same computation.

Here: compare the "tape" of `PUT32`/`GET32` calls. Same sequence of
"loads" and "stores" → same behavior.  We can ignore the code.

**What you get:**

- Confidence your code works.
- Automatic detection when future changes break things.
- A technique you'll use constantly this quarter (and beyond).
- (Maybe) Subtle: given that you have a solid safety net you can make
  tricky changes without stress.  Refactor, extend, speed hacks: Just
  make sure you get the same checksums!  

  Example from today: extension where you write fast, inlined GPIO
  versions that do not have error checking.  We use these alot in
  240lx/340lx to write timing-critical code: easy 2x with significantly
  lower jitter.  With cross checking takes less than 10 stress free
  minutes to do.

----------------------------------------------------------------------
### TL;DR: Checkoff crash course

**What you'll do (4 steps):**

1. Compile and link your `gpio.c` against the `1-fake-pi` simulator on 
   Unix → logs all PUT32/GET32 calls
  - Compare checksums with partner and class
2. Implement `gpio_set_function(pin, func)` (generic pin configuration)
  - Compare checksums with partner and class
3. Run same code on real pi with hardware tracing (linker `--wrap` trick)
  - Compare checksums against checksums from (2).

**Success criteria:**

**Part 1 - Fake Pi (Unix):**
```bash
% cd 1-fake-pi/tests
% make checkoff
```

Success looks like:
```
total files = 29
USE THIS VALUE FOR CHECKOFF:
525118589 1049
```

**Quick debug tip:**

Part 1: If checksums differ in (`1-fake-pi/tests`): 
1. Start with `0-*` tests (single GPIO calls, easiest to debug)
2. Then `1-*` tests (loops over all pins)
3. Then `2-*` and `5-*` tests (illegal pins, gpio_set_function)
4. Finally `prog-*` and `act-*` tests (full programs)

Compare individual `.out` files with your partner to find where you diverge.

**Final checkoff value:** `525118589`

If you match this, you're done with Part 1.


**Part 2 - Hardware Tracing (Real Pi):**
```bash
% cd 2-trace/tests
% make emit
% make check
```

Success looks like: all tests pass, checksums match your Part 1 checksums.


**Part 3 - Integration (quick):**
```bash
% cd libpi/
% # Edit Makefile: change SRC = src/gpio.c, comment out `# staff-objs/gpio.o`
% cd ../2-trace/tests
% make check
```

Success looks like: checksums stay the same as Part 2 (your GPIO works correctly).

**Many Extensions (check back: updating):**
- GPIO pullup/pulldown (see PRELAB.md)
- Array-based memory in fake-pi (instead of globals)
- Write tests that catch bugs we missed

### Staff Checksums (Reference)

When you're debugging, you can compare individual test checksums against ours:

```bash
% cd 1-fake-pi/tests
% make checkoff
```

**Our reference output:**
```
[Check back: UPDATING]
[Check back: UPDATING]
[Check back: UPDATING]
[Check back: UPDATING]
...
total files = 29
USE THIS VALUE FOR CHECKOFF:
525118589 1049
```

**How to use this:**

You don't need all checksums to match immediately. Work incrementally:
1. Get `0-*` tests matching first (simplest, single calls)
2. Then `1-*` tests (loops, systematic)
3. Then `2-*` tests (illegal inputs)
4. Then `5-*` tests (gpio_set_function)
5. Finally `act-*` and `prog-*` tests (full programs)

If a specific test differs, you can open the `.out` file to see the
exact sequence of `PUT32`/`GET32` calls:
```bash
% cat 1-fake-pi/tests/0-gpio-read-17.out
```

----------------------------------------------------------------------
### How to Submit (Autograder)

***NOTE: updating this***


----------------------------------------------------------------------
### 1. Check your code against everyone else (`1-fake-pi`)

To repeat: A simple, stringent approach to check that two `gpio`
implementations are the same:
  1. They read and write the same addresses in the same order with the 
     same values.
  2. They return the same result.

If all checks pass then we know both implementations are equivalent in 
how they read or write device memory --— at least for the tested inputs.

You won't write much code for this part. Most of the work is
comparing your `PUT32` and `GET32` traces when running the tests in
`1-fake-pi/tests` against everyone else to find differences.

##### One-time setup.

**Step 1: Copy your gpio.c**

Don't break working code. Copy your gpio.c from Lab 2 and
make sure it compiles:
```
% cd 1-fake-pi
% cp ../../2-gpio/code/gpio.c .
% make
```

**Step 2: Run a single test**

```
% cd tests
% make run        # should run a single test
% make check      # test should pass

checking 0-gpio-write-17:
about to emit a new test output <0-gpio-write-17.test>:
    ./0-gpio-write-17.fake > ./0-gpio-write-17.test
about to compare...
Success!  Matched!
```

**Step 3: Read the Makefile**

Look in `tests/Makefile` to see how to run other tests.

##### Workflow

**Test organization (easiest to hardest):**

- `0-*` tests: Single legal GPIO call (start here, easiest to debug)
- `1-*` tests: Loop all legal pins < 32
- `2-*` and `5-*` tests: Many pins, both legal and illegal
- `prog-*` tests: Full programs from last lab
- `act-*` tests: ACT LED tests

Control which tests run by setting `TEST_SRC` in `1-fake-pi/tests/Makefile`:

```make
TEST_SRC := $(wildcard ./[0]-*.c)    # run 0-* tests
TEST_SRC := $(wildcard ./[1]-*.c)    # run 1-* tests
```

Makefile variables: last write wins. You don't need to comment out
previous writes.

**The Makefile Commands:**

- `make run`: Compile and run tests (checks for crashes)
- `make emit`: Run tests, saves output to `.out` files (used for
  regression and cksum)
- `make check`: Rerun tests, compare to previously emitted `.out` files
  (use after any `gpio.c` modification)
- `make cksum`: Run `cksum` on every `.out` file (compare to partners)
- `make checkoff`: Compute all checksums for final checkoff

**Typical workflow:**

1. Set `TEST_SRC`: start with easiest tests first (`0-*`)
2. `make run` to check for crashes
3. `make emit` to save output
4. `make cksum` to compare with partner
5. If different: open individual `.out` files to see where you diverge
6. Fix your code, `make check` to verify old tests still pass
7. Move to next test set, repeat

You should compare with your partner and work through the tests.

##### Making  code behave the same on illegal inputs

The `0-` and `1-` tests work on legal inputs. The `2-*.c` tests check 
illegal pin inputs.


**Why this matters for equivalence:** Illegal inputs often have no
defined result. Different implementations could reasonably do different
things. To make equivalence checking work, all your GPIO routines should
immediately panic if an input pin is greater than 53 (`GPIO_MAX_PIN`):
```c
    if(pin > GPIO_MAX_PIN)
        gpio_panic("illegal pin=%d\n", pin);
```

**Why this matters for safety:** Checking illegal pins is not a priggish
"nice have" --- it's a hard "don't die" requirement. Fed an illegal pin
your GPIO code will compute an unexpected address, which it then nukes
with invalid loads and stores.  Possible results: corrupting data, code
(we have no memory protection yet!) or device configs; destroying device
FIFOs and their data, etc.

----------------------------------------------------------------------
#### Step 2: implement `gpio_set_function` and cross-check it

Debugging without even `printk` is a hassle.  So we'll fix that now.

**What to implement:**

```c
// set GPIO function for <pin> (input, output, alt...).
// settings for other pins should be unchanged.
void gpio_set_function(unsigned pin, gpio_func_t function);
```

This is a more generic version of your `gpio_set_output` and
`gpio_set_input`. It takes an `enum` constant (see `gpio.h`) telling it
how to configure the pin (see the Broadcom document for their definitions).

**Implementation:**

  1. Only modify your `gpio.c` you copied from Lab 2 to `1-fake-pi`.
  2. Adapt your `gpio_set_output` code to bitwise-or the given flag.
  3. Error check both the input `pin` and the `function` value.

**Checkoff:**

  1. Make sure the `5-tests*.c` are equivalent to other people
  2. **Afer** doing (1): then rewrite your `gpio_set_input` and
     `gpio_set_output` to call `gpio_set_function` and verify you get the
     same checksums (see a pattern?)
  3. Check that `printk` now works: `cd 00-hello`, run `make`, verify
     `hello.bin` runs successfully

If step 3 works, congratulations — it's using your `gpio.c`.

**Why printk now works:** The UART code (used by `printk`) needs to
configure GPIO pins using alternate functions. Your `gpio_set_function`
provides this.

**Why printk**: Like many other kernels, we call our `printf` "`printk`"
because it's privileged (and so errors can crash the machine) and,
related, because its implementation is missing most of `printf`'s
functionality.

-------------------------------------------------------------------
#### Step 3: extend `fake-pi.c` to handle the act led (pin 47).

The pi has an led on its board (the "act" led) that you can control
by writing GPIO pin 47.

For `fake-pi.c`:
  - You will have to extend `fake-pi.c` to have additional clear
    (`gpio_clr1`), set (`gpio_set1`) and function select location
    (`gpio_fsel4`).

  - extend `fake-pi.c:GET32`: to handle reads of the new function select location.
  - extend `fake-pi.c:PUT32`: to handle writes to the new clear and set locations.
  - do not add calls to initialize the new variables in `notmain` using
    random!  It will throw off the values.  You'll notice `gpio_fsel4_v` is
    set to `~0`.

For testing:
  - Enable the act tests:

```
        # 1-fake-pi/tests/Makefile
        TEST_SRC := $(wildcard ./act-*.c)
```

-------------------------------------------------------------------
## Final checkoff for part 1.

Now that you're done, for checkoff:

  - run `make  checkoff` which will compute a checksum of all
     the checksums.  You should get a total of XX  files, and the
     checksum should be `XXX`.  (***WE WILL UPDATE THIS.***)

---------------------------------------------------------------
#### Part 2: Do similar tracing on the pi (`2-trace/`)

***IF YOU SEE THIS DO A git pull***
***IF YOU SEE THIS DO A git pull***
***IF YOU SEE THIS DO A git pull***
***IF YOU SEE THIS DO A git pull***
***IF YOU SEE THIS DO A git pull***
***IF YOU SEE THIS DO A git pull***



***Note: for the `prog-hardware-loopback.c` you'll need to run a jumper
between pins 8 and 9 (it sets and reads).***  

Part 2 uses the tracing trick from the `PRELAB`.  You should look at
that implementation in `01-tracing-pi/` if you haven't already.

**What to implement in the `2-trace/` directory:**

  - `trace-simple.c`: implement `__wrap_PUT32` and `__wrap_GET32`
  - `trace-notmain.c`: if you want to get fancy, implement this so you can run raw pi programs in tracing mode
  - `tests/Makefile`: extend to handle `get32` and `put32` (just change your trace library to call the wrappers)

As with `1-fake-pi`, start working through the tests in `2-trace/tests`.

##### Checkoff

Initially you'll be using our `gpio` implementation in `libpi`. When
you finish the tracing above, emit the `out` files and then drop in your
gpio and make sure you get the same answer.

**Steps:**

Before changing anything:
   1. Verify `make` in the test directory
      works. `hello-bin.bin` should print "hello" and `act-blink.bin`
      should blink the small green led on the pi itself.

   2. `make emit` to emit all the `*.out` files recording the reference
      PUT32 and GET32 calls.

   3. `make check` to sanity check that the unchanged code produces
      the same output files as those emitted in step 1 above.  (This detects
      non-determinism or broken checking code).

Now test your code works when used as the default:

   1. Swap your `gpio.c` into `libpi/` by changing the `libpi/Makefile` to 
      use your `gpio.c` instead of ours:
      - Add: `SRC = src/gpio.c`
      - Remove `staff-objs/gpio.o` from `STAFF_OBJS` by commenting it out.
      - These steps are described in the `libpi/Makefile` comments

   2. `make check`: after swapping, verify tracing gives the same values.

----------------------------------------------------------------------
#### Extension: simulator validation

Modify the `fake-pi.c` implementation to set memory to the actual values 
on the pi when called with `--initial`. You should then check that you
get the same results when run on the same program. Note: you will have
to do something about the tracing start/stop calls.


----------------------------------------------------------------------
#### Extension: Implement a better version of memory that uses an array

Our crude memory in `fake-pi.c` is relatively simple to understand,
but very rigid since it uses a global variable for each device memory
address. Make a copy of code so you don't break your working version
(`cp -r code code-new`) and reimplement memory using an array. When you
rerun all the tests (`make check`) nothing should change.

----------------------------------------------------------------------
#### Extension: write other tests

We'll give an extension credit if you can write a test that detects
a new difference that the provided tests miss.

-------------------------------------------------------------------------
#### Post-script: the power of fake execution + equivalence

It doesn't look like much but you've implemented a fancy approach to 
correctness that is --- hard to believe --- much beyond what almost
anyone does. There are two pieces to what you've done:

    1. **Taking low-level code and running it in a fake environment.**
    You took code made to manipulate hardware directly and ran it
    without modification in a fake environment. This approach to lying
    (more politely called "virtualization") is powerful and useful in
    many domains. In an extreme case you have virtual machines, such as
    VMware, which can take entire operating systems designed to run in
    god-mode on raw hardware (such as your laptop) and instead run them as
    user-processes that deal with fake hardware without them realizing it.


    2. **Showing code equivalence via memory traces.** You checked
    equivalence not by comparing the code itself (which is hard when
    code differs even trivially, much less dramatically) but instead
    by checking that it does equivalent reads and writes. Checking
    memory-access equivalence gives us a simple, powerful method to
    mechanically check code correctness. In our case, it lets us show
    that you are equivalent to everyone else by comparing a single 32-bit
    number. Even better: if even one implementation is correct, we have
    trivially shown they all are -- at least on the runs we have done ---
    no matter how weirdly you wrote the code.  Or even in what language!
    Several people are doing rust versions, and a couple of maniacs are
    doing zig: your hashes cross-check just as well against their code
    as against a C implementation.

Showing *exact* equivalence of a specific run is easy:  a `strcmp` of the
output is sufficient.   It does have a couple of challenges.

   1. **Limited test coverage.** It only shows equivalence on the
   inputs we have tested. It cannot show that there is not some other
   input that causes a non-equivalent execution. We *can* significantly
   extend this by checking the code symbolically (essentially running
   it on constraints expressing all possible values on a path vs just
   running it on a single set of concrete values), but this is beyond
   the scope of the course. A bunch of my former students had a bunch
   of great papers on this approach if interested.

   2. **False rejections from reordering.**  Our current exact-match
   approach will reject semantically equivalent runs that have superficial
   differences. For example, consider code where one implementation first
   sets GPIO pin 19 to an output pin, and then pin 20 while another does
   the reverse. Intuitively these are the same, since it does not matter
   which order you set these pins, but since these reads and writes will
   be in different orders we will reject these.  You should be able to
   see how to extend to handle this --- and you can certainly do so as
   an extension!
