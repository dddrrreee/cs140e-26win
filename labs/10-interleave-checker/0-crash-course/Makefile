# PROGS += 0-nop1-example.c
# PROGS += 1-many-fn-example.c
# PROGS += 2-lib.c
# PROGS += 3-gproffed.c
PROGS += 4-bad-addr-checker.c
# PROGS += single-step.c

COMMON_SRC += single-step.c
# COMMON_SRC += ../../11-debug-hw/code/breakpoint.c


L := $(CS140E_2026_PATH)/libpi/
STAFF_OBJS += $(L)/staff-objs/staff-kmalloc.o
STAFF_OBJS += $(L)/staff-objs/staff-full-except-asm.o
STAFF_OBJS += $(L)/staff-objs/staff-switchto-asm.o
# STAFF_OBJS += $(L)/staff-objs/staff-breakpoint.o
# STAFF_OBJS += ../../11-debug-hw/code/staff-breakpoint.o
STAFF_OBJS += $(L)/staff-objs/staff-full-except.o

BOOTLOADER=my-install
RUN = 1
CAN_EMIT=1

OUR_START = single-step-start.S

# you can change opt level and the runs won't change.
# OPT_LEVEL = -O2

EXCLUDE ?= grep -v simple_boot

GREP_STR := 'TRACE:\|SUCCESS:\|ERROR:\|PANIC:'
include $(CS140E_2026_PATH)/libpi/mk/Makefile.robust-v2
