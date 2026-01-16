### Testing your gpio.c using fake-pi

This directory contains a fake r/pi system that lets you debug your
`gpio.c` code on your laptop. You compile it along with `gpio.c` and test
programs to produce executables that run on your laptop and print all
`PUT32`/`GET32` calls.

**Background:** See `../BACKGROUND.md` for the conceptual explanation of how
this works and why it's useful.

---

### Quick start

1. **Copy your `gpio.c` into this directory:**
   ```bash
   % cp ../../2-gpio/code/gpio.c .
   ```

2. **Build the library:**
   ```bash
   % make
   ```

3. **Run tests:**
   ```bash
   % cd tests
   % make run      # Run all tests
   % make cksum    # Compute checksums to compare with others
   ```

---

### What you get

**Benefits of running on Unix:**
1. You can run the "pi" program using a **debugger** (gdb, lldb)
2. You have **memory protection** --- null pointer writes get detected
3. You can run with **tools** (e.g., valgrind) to look for other errors
4. **Fast compile/test cycles** --- no bootloader delays

**Equivalence checking:**
By comparing the actual `PUT32`/`GET32` values you can check your code
against other people. If your sequence is identical to theirs, that
proves your code is equivalent (on that run) to theirs, despite the two
implementations looking very different.

---

### How it works

The file `fake-pi.c` implements fake `PUT32` and `GET32` functions that:
- Track all GPIO register addresses using enums and global variables
- Print every `PUT32`, `GET32` call.
- Treat most GPIO memory as "regular" memory (read returns last write)
- Special case: `gpio_lev0` returns pseudo-random values (simulates external input)

Most of your code is straight C, which runs the same on your laptop and
the r/pi. The only pi-specific parts are:
- Assembly code in `start.S` (which we replace with C versions)
- GPIO register access (which we intercept via fake `PUT32`/`GET32`)

See `fake-pi.c` for implementation details.

---

### Testing workflow

The `tests/` directory has tests organized by difficulty:
- `0-*.c`: Easiest tests (single simple operations)
- `1-*.c`: Multiple calls, possibly with illegal inputs
- `2-*.c` and higher: More complicated scenarios
- `act-*.c`: Requires adding addresses to `fake-pi.c`
- `prog-*.c`: Full programs from 2-gpio.

#### Select which tests to run

Edit `tests/Makefile` and modify the `TEST_SRC` variable:

```makefile
# Run a single test (start here):
TEST_SRC := 0-gpio-write-17.c

# Run all 0-* tests (easy):
TEST_SRC := $(wildcard ./[0]-*.c)

# Run all 1-* tests (harder):
TEST_SRC := $(wildcard ./[1]-*.c)

# Run everything:
TEST_SRC := $(wildcard ./*.c)
```

#### Test commands

From `tests/` directory:

```bash
# Simply run all selected tests
% make run

# Run and compute checksums (for comparing with others)
% make cksum

# Generate .out files for later comparison
% make emit

# Re-run and check against previously generated .out files
% make check
```

#### Recommended workflow

1. **Start simple:** Run a single easy test (e.g., `0-gpio-write-17.c`)
2. **Compare with others:** For detailed checking look at the associated
   `.out` file (e.g. `0-gpio-write-17.out`).  For quick high level
    checking use `make cksum` and post results to compare 
3. **Save results:** `make emit` to save the outputs
4. **Progress to harder tests:** Move through `0-*.c`, `1-*.c`, `2-*.c`, etc.
5. **Check for regressions:** After making changes, `make check` verifies old
   tests still pass
6. **Final checkoff:** Compute checksum of all outputs:
   ```bash
   % make checkoff
   ```

---

### Example: Running a single test

```bash
% cd tests
% make
% ./0-gpio-write-17.fake
calling pi code
... initialization ...
TRACE:1: GET32(0x20200008) = 0x66334873
TRACE:2: PUT32(0x20200008) = 0x66334871
TRACE:3: PUT32(0x2020001c) = 0x100000
...
TRACE: pi exited cleanly: 7 calls to random
```

Each `TRACE` line shows exactly what your code did. If you and your partner
get identical traces, your implementations are equivalent.

To compare:
```bash
% ./0-gpio-write-17.fake > 0-gpio-write-17.out
% cksum 0-gpio-write-17.out
1234567890 123 0-gpio-write-17.out   # Compare this number with others
```

---

### Extending fake-pi.c

You may need to add addresses to `fake-pi.c` for some tests (especially
`act-*.c` tests). Look for the `enum` of tracked addresses and the
corresponding variables, then add cases to the switch statements in
`PUT32` and `GET32`.

---

### Files in this directory

- **`fake-pi.c`**: The fake implementation (enum/switch for GPIO addresses)
- **`fake-random.c`**, **`pi-random.c`**: Pseudo-random number generator
  (ensures everyone gets same "random" values)
- **`gpio.c`**: Your implementation (copy from lab 2)
- **`Makefile`**: Builds `libpi-fake.a`
- **`tests/`**: Directory with test programs
- **`tests/Makefile`**: Test automation
- **`tests/Makefile.test`**: Test infrastructure (called by tests/Makefile)

---

### Tips

- If tests crash with segfault: **Good!** Your code has a bug that memory
  protection caught. Use gdb to find it.
- If your checksum differs from others: At least one person has a bug.
  Compare the TRACE output line-by-line to find where they diverge.
- If you're stuck: Start with the simplest tests and work up. The `0-*.c` tests
  call each function once with simple inputs.
