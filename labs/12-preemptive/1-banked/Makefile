# TODO:
#   1. Run "make check" before modifying anything to make 
#      sure works.
#   2. Implement the code in <banked-set-get-asm.S>
#   3. Make sure "make check" still passes.
PROGS  += 0-any-mode-sp-lr.c
PROGS  += 0-mem-user-sp-lr.c
PROGS  += 0-user-sp-lr.c
# start with this one!
PROGS  += 0-cps-user-sp-lr.c

# Put your code in <banked-set-get-asm.S>
# COMMON_SRC += banked-set-get-asm.S  
STAFF_OBJS += staff-banked-set-get-asm.o

# change this if you want!
BOOTLOADER=my-install

# RUN=1 ==> automatically run.
RUN=1
CAN_EMIT=0
EXCLUDE ?= grep -v simple_boot
include $(CS140E_2026_PATH)/libpi/mk/Makefile.robust-v3
