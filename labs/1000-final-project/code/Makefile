COMMON_SRC += net-stack/netif/w5500.c net-stack/inet.c

# PROGS += tests/playground-nrf.c
# PROGS += tests/playground-w5500.c
# PROGS += tests/playground-driver.c
# PROGS += tests/playground-frame-testing.c
PROGS += tests/playground-rx.c

# next week you'll get rid of all of these
O = $(CS140E_2026_PATH)/libpi
STAFF_OBJS += $(O)/staff-objs/staff-full-except.o
STAFF_OBJS += $(O)/staff-objs/staff-full-except-asm.o
STAFF_OBJS += $(O)/staff-objs/staff-switchto-asm.o
STAFF_OBJS += $(O)/staff-objs/staff-kmalloc.o
STAFF_OBJS += $(O)/staff-objs/staff-hw-spi.o

BOOTLOADER = my-install
TTYUSB = 
RUN = 1
include $(CS140E_2026_PATH)/libpi/mk/Makefile.robust-v3

clean::
	make -C tests clean
