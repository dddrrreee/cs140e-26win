PROGS := test-mbox.c
PROGS := test-pl011.c
COMMON_SRC := mbox.c

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = pi-install
BOOTLOADER = my-install
RUN=1

# if you need pl011
L = $(CS140E_2026_PATH)/libpi/staff-objs
STAFF_OBJS += $(L)/pl011-uart.o


# pull in libgcc in case you need division.
LIBGCC  = $(shell $(CC) -print-libgcc-file-name)
LIB_POST += $(LIBGCC)

include $(CS140E_2026_PATH)/libpi/mk/Makefile.robust-v2
