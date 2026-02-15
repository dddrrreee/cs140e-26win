# simple definitions for making libpi.a
#  - you should only ever have to modify SRC or STAFF_OBJS

SRC += src/gpio.c
SRC += src/uart.c
SRC += src/gpio-int.c
SRC += src/sw-uart.c
SRC += src/watchpoint.c
SRC += src/breakpoint.c
# SRC += src/single-step.c

# if you want to use your own pullup-puldown
# 1. put it in src/gpio-pud.c)
# 2. add it to SRC
# 3. comment out ours below:
STAFF_OBJS  +=  ./staff-objs/gpio-pud.o

# these are all the locations that get made into
# libpi.a
#
# you should not have to modify this variable.  
SRC :=  $(SRC)                              \
        $(wildcard ./libc/*.[Sc])           \
        $(wildcard ./staff-src/*.[Sc])      \
        staff-start.S

# hack to minimize git conflicts: we do various customizations
# in there; but probably would be clearer to inline it.
include ./manifest.mk
