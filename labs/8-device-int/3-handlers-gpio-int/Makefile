# to use your gpio-int.c swap these.

# swap these two to use your code.
# COMMON_SRC := ../2-gpio-int/gpio-int.c
STAFF_OBJS += $(CS140E_2026_PATH)/libpi/staff-objs/gpio-int.o

# swap these two to use your code 
# COMMON_SRC := test-interrupts.c
STAFF_OBJS += staff-test-interrupts.o

########################################################
# tests: 3-* hardest, 2-* easier, 1-* easier.  
# run 0-check-connect.c first

# level 3: do N iterations of event type.
# easiest: one type of event.
# PROGS += 3-N-rising-edge.c
# harder: rise + fall
# PROGS += 3-N-rise-fall.c
# hardest: everything.
# PROGS += 3-N-rise-fall-timer.c

# level 2: combine more than one event type
# easier
# PROGS += 2-rise-fall-edge.c
# has timers so is harder.
# PROGS += 2-timer-rise-fall-edge.c

# level 1: easiest: do a single event once 
# (other than for timer)
# PROGS += 1-falling-edge.c
# PROGS += 1-rising-edge.c
# PROGS += 1-timer.c

# simplest: make sure that loopback works.
PROGS += 0-check-connect.c

# run everything
PROGS = $(wildcard [0123]-*.c)

# given
COMMON_SRC += interrupt-asm.S 

# uncomment if you want it to automatically run.
RUN=1

CAN_EMIT=0
EXCLUDE ?= grep -v simple_boot
include $(CS140E_2026_PATH)/libpi/mk/Makefile.robust-v2
