# Makefile for 1-fake-pi: builds libpi-fake.a for running r/pi code on Unix

LIBNAME = libpi-fake.a
LPP = $(CS140E_2026_PATH)/libpi

# Source files for fake-pi library
LIB_SRC  = fake-pi.c fake-random.c pi-random.c
LIB_SRC += $(LPP)/libc/printk.c $(LPP)/libc/putk.c $(LPP)/libc/putchar.c
LIB_SRC += $(LPP)/staff-src/clean-reboot.c
LIB_SRC += gpio.c

# Include directories
INC = -I$(LPP)/include -I$(LPP)/libc -I.

include $(CS140E_2026_PATH)/libunix/mk/Makefile.lib.template

# Export includes for tests to use
all::
	@echo "INC += $(INC)" > includes.mk

# if <gpio.c> not in this directory give an error.
gpio.c:
	$(error "copy 2-gpio/code/gpio.c into this directory")

checkoff:
	@make -C tests checkoff

clean::
	rm -f includes.mk
