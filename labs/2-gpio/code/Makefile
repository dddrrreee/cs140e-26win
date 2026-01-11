$(error "don't modify the code here: make a copy if you want to modify)

# simple, self-contained Makefile for today's lab's programs.
# you should read through and understand how it works!

# the programs we build.  since each program starts with a number
# and no other file does, we could automatically get them using 
# a regular expression:
#   PROG_SRC = $(wildcard [0-9]-*.c)
# but we list them out explicitly to simplify things.
PROG_SRC = 1-blink.c  2-blink.c 3-loopback.c 4-act-blink.c 

# the object files each program depends on.
objs := start.o gpio.o delay.o  mem-barrier.o

# the name of the stripped binaries we make: these can be run
# using the bootloader.
progs   := $(PROG_SRC:.c=.bin)
# each .bin file has a .list file containing a human-readable
# disassembly of its machine code
lists   := $(PROG_SRC:.c=.list)
# the elf files used to generate the .bin 
elfs    := $(PROG_SRC:.c=.elf)

# linker script to lay out the executable 
MEMMAP := ./memmap

# dependencies: recompile everything if any of these change.
DEPS := ./Makefile  rpi.h

# disable built-in rules.
.SUFFIXES:

# short-hand variables for the cross-compilation tools we use
ARM := arm-none-eabi
CC  := $(ARM)-gcc
LD  := $(ARM)-ld
AS  := $(ARM)-as
OD  := $(ARM)-objdump
OCP := $(ARM)-objcopy
AS  := $(ARM)-as

# The gcc compiler flags:
#    -Wall: warn about a bunch of suss things.
#    -Werror = must fix errors
#    -O: optimize (which allows for more -Wall error checking)
CFLAGS := -O -Wall -Wno-unused-variable -Werror \
          -nostdlib -nostartfiles -ffreestanding  -march=armv6 -std=gnu99  

# the assembler flags (for start.S)
ASFLAGS = --warn --fatal-warnings -mcpu=arm1176jzf-s -march=armv6zk

# our target: all the .bin files.
all:  $(progs)

# rules to generate .elf->.list->.bin
$(elfs): %.elf: %.o $(objs)  $(DEPS)  $(MEMMAP)
	$(CC) $(CFLAGS) $(objs) $< -T $(MEMMAP) -o $@ 

# generate a .list file from an elf file so 
# we can see what the machine code looks like
$(lists): %.list: %.elf
	$(OD) -D  $< > $@

# generate a .bin file from each .elf
$(progs): %.bin: %.list %.elf
	$(OCP) $*.elf -O binary $*.bin

# how to generate .o's from the .c's.
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# how to generate .o's from .S's
%.o: %.S $(DEPS)
	$(AS) -c $(ASFLAGS) $< -o $@

clean:
	rm -f *.bin *.list *.elf *.o Makefile.bak *~

.PHONY: all clean 

# cheat and just keep all the .o's.
.PRECIOUS: %.o
