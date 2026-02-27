# you'll get rid of this
STAFF_OBJS += staff-nrf-driver.o

# the NRFs can be finicky about back to back runs.
# if you want to go faster --- get rid of the sleep.
BOOTLOADER = sleep 1; my-install


# easiest
# PROGS := 0-no-ack-check.c
# then.
# PROGS := 0-ack-check.c

# ordered easy to harder
PROGS += $(wildcard tests/0-*.c)
PROGS += $(wildcard tests/1-*.c)
PROGS += $(wildcard tests/2-*.c)
PROGS += $(wildcard tests/3-*.c)

# starter code.  
COMMON_SRC := nrf-hw-support.c nrf-public.c nrf-driver.c

STAFF_OBJS += $(CS140E_2026_PATH)/libpi/staff-objs/staff-hw-spi.o
STAFF_OBJS += $(CS140E_2026_PATH)/libpi/staff-objs/staff-kmalloc.o

# uncomment if you want it to automatically run.
RUN=1

EXCLUDE ?= grep -v simple_boot
GREP_STR := 'HASH:\|ERROR:\|PANIC:\|SUCCESS:\|NRF:'
include $(CS140E_2026_PATH)/libpi/mk/Makefile.robust-v3

checkoff:
	make -f ./Makefile PROGS="$(wildcard tests/[0123]-*.c)" check
	# make -f ./Makefile PROGS="$(ALL)" check
