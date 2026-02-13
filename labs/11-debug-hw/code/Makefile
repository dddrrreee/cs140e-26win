# these are all the tests.
# PROGS :=  0-example-debug-id.c $(wildcard [012]*-*test.c)

PROGS += 2-match-test.c  
PROGS += 1-watchpt-byte-test.c  
PROGS += 1-watchpt-test.c  
# simplest: should run out of the box.
PROGS += 0-example-debug-id.c

# same start as last lab: used so we have known routines in
# known locations.
OUR_START = single-step-start.S

# you should delete these when done.
STAFF_OBJS += staff-watchpoint.o
STAFF_OBJS += staff-breakpoint.o

COMMON_SRC += breakpoint.c
COMMON_SRC += watchpoint.c
# COMMON_SRC += mini-watch.c
# COMMON_SRC += mini-step.c

# you'll be writing these next week.
L := $(CS140E_2026_PATH)/libpi/
STAFF_OBJS += $(L)/staff-objs/staff-kmalloc.o
STAFF_OBJS += $(L)/staff-objs/staff-full-except-asm.o
STAFF_OBJS += $(L)/staff-objs/staff-switchto-asm.o
STAFF_OBJS += $(L)/staff-objs/staff-full-except.o

# define this if you need to give the device for your pi
TTYUSB = 
# if your bootloader has issues just use pi-install.
BOOTLOADER = pi-install
BOOTLOADER = my-install

# set to 0 if you don't want it to run
RUN=1

CAN_EMIT=0
GREP_STR := 'TRACE:\|THREAD:\|ERROR:\|PANIC:' 
include $(CS140E_2026_PATH)/libpi/mk/Makefile.robust-v2
