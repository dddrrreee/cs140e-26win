# Makefile.tutorial -- a tiny guided tour of how this Makefile works.
# Read top to bottom; every variable is used later in a rule.

# 1) What are we building?
# The programs used in today's lab: each has a notmain.
# NOTE: this variable and many of the others are not required:
# we use them to (try to) add clarify.  The only real requirement
# is specifying what depends on what (the rules from "all:" to EOF).
PROG_SRC := 1-blink.c 2-blink.c 3-loopback.c 4-act-blink.c 5-all.c 6-uart.c 7-pullup.c 8-pullup-with-uart.c

# 2) For each PROG_SRC X.c we build a X.bin
PROGS := $(PROG_SRC:.c=.bin)

# These common object files are linked into every program
# so we track them using a variable
OBJS := start.o gpio.o libpi.o uart.o

# The linker script decides where code/data go in memory.
MEMMAP := ./memmap

# 3) Cross‑compiler toolchain: your laptop probably does not
# have the same CPU as the pi, so we have to "cross-compile"
# using a different compiler than your native one.  You set
# this up in lab: 0-setup-pi.
ARM     := arm-none-eabi
CC      := $(ARM)-gcc

# 4) Compiler flags.
# gnu99 + error checking: don't compile if with warnings.
CFLAGS := -O -Wall -Wno-unused-variable -Werror -std=gnu99 
# no C runtime or libc.
CFLAGS += -nostdlib -nostartfiles -ffreestanding 
# our pi zero w runs on an ARM1176JZF-S (ARMv6ZK) processor
CFLAGS += -mcpu=arm1176jzf-s
# avoids unaligned access traps.
CFLAGS += -mno-unaligned-access

# 5) The default target: build all binaries.
all: $(PROGS)

# 6) The rule for building a .bin.  We incidentally also
# produce a .elf and .list (disassembled elf).
%.bin: %.o $(OBJS) $(MEMMAP) Makefile rpi.h
	# $* is the shared name: for target 1-blink.bin, $* == 1-blink
	# $< is the first input file (the .o built from 1-blink.c)
	$(CC) $(CFLAGS) $(OBJS) $< -T $(MEMMAP) -o $*.elf
	$(ARM)-objcopy $*.elf -O binary $@
	$(ARM)-objdump -D $*.elf > $*.list

# 7) The rule for building a .o from a .c C file.  The 
# rule states that every .o will get remade if its 
# .c (same name), rpi.h, or Makefile changes (is newer
# then the .o)
%.o: %.c Makefile rpi.h
	$(CC) $(CFLAGS) -c $< -o $@

# 8) The rule for building a .o from a .S file. Similar 
# dependencies.  Note: uppercase .S means the C preprocessor
# runs first, b/c we use gcc we can use macros in the .S (v handy).
%.o: %.S Makefile rpi.h
	$(CC) $(CFLAGS) -c $< -o $@

# 9) "make clean": Delete all generated files (not sources).
clean:
	rm -f *.bin *.list *.elf *.o Makefile.bak *~

# 10) tell `make` these are fake targets (do not generate a file)
# to reduce some confusing errors.  (Won't arise today, but easily
# can in a larger project.)
.PHONY: all clean
