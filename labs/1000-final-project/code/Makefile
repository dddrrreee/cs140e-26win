COMMON_SRC += net-stack/netif/w5500.c 

# Layer 2
COMMON_SRC += net-stack/data-link.c 
COMMON_SRC += net-stack/arp.c

# Layer 3
COMMON_SRC += net-stack/network.c 
COMMON_SRC += net-stack/icmp.c

# Tests
PROGS += tests/1-test-arp-table.c
PROGS += tests/1-test-fill-arp-table.c
# PROGS += tests/1-test-send-resolved-arp.c

# Playgrounds
# PROGS += tests/0-playground-nrf.c
# PROGS += tests/0-playground-w5500.c
# PROGS += tests/0-playground-driver.c
# PROGS += tests/0-playground-frame-testing.c
# PROGS += tests/0-playground-rx.c
# PROGS += tests/0-playground-net-stack.c


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
