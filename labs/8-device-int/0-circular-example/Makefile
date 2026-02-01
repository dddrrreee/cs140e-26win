PROGS += 0-circular-example.c 
PROGS += 1-circular-example32.c

# if you want to use your libpi
# LPI = ../staff-objs/libpi.a
L = $(CS140E_2026_PATH)/libpi/
STAFF_OBJS += $(L)/staff-objs/interrupts-asm.o

# automatically run the code
RUN = 1

# if you need to override the install
BOOTLOADER = my-install

include $(CS140E_2026_PATH)/libpi/mk/Makefile.robust-v2
