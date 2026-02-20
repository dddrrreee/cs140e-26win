# TODO:
#   1. Run "make check" before modifying anything to make 
#      sure works.
#   2. Implement the code in <banked-set-get-asm.S>
#   3. Make sure "make check" still passes.
PROGS  += 1-rfe-example.c
COMMON_SRC += 1-rfe-asm.S

# change this if you want!
BOOTLOADER=my-install

# RUN=1 ==> automatically run.
RUN=1
CAN_EMIT=0
EXCLUDE ?= grep -v simple_boot
include $(CS140E_2026_PATH)/libpi/mk/Makefile.robust-v3
