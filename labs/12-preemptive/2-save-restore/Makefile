# change this if you wanna run automatically
RUN = 0
# when done: remove this.
STAFF_OBJS += staff-switchto-asm.o
# when done: remove this.
STAFF_OBJS += staff-breakpoint.o

PROGS += 2-simple-save-test.c	 
PROGS += 3-simple-restore-test.c  
PROGS += 3-multi-restore-test.c	 

# you have to implement these!
# PROGS += 4-match-restore-test.c  
# PROGS += 4-multi-match-test.c	 

PROGS += 5-match-priv-test.c
PROGS += 5-multi-priv-test.c

COMMON_SRC += interrupt-asm.S
COMMON_SRC += switchto-asm.S

OUR_START = start.S

L := $(CS140E_2026_PATH)/libpi/
STAFF_OBJS += $(L)/staff-objs/staff-kmalloc.o

BOOTLOADER=my-install

CAN_EMIT=0
EXCLUDE ?= grep -v simple_boot
GREP_STR := 'TRACE:\|SUCCESS:\|ERROR:\|PANIC:'
include $(CS140E_2026_PATH)/libpi/mk/Makefile.robust-v3

clean::
	rm -f *.raw
