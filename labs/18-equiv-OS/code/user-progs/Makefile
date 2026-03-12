# the tests in decreasing order of difficulty.
PROGS := 0-hello.c 1-fork.c 0-printk-hello.c 1-fork-waitpid.c

# a list of all of your object files.

# external-code/bzt-sd.c 

COMMON_SRC := libos-cstart.c # libos-asm.S
CFLAGS += -I..
START = libos-asm.o

TTYUSB = 
BOOTLOADER = my-install
RUN = 1

MEMMAP := small-proc.ld
RUN = 0

include $(CS140E_2024_PATH)/libpi/mk/Makefile.template-v3

hello.o:
