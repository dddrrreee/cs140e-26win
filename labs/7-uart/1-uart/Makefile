###############################################################
# MAKE SURE TO UNCOMMENT the line:
# MAKE SURE TO UNCOMMENT the line:
# MAKE SURE TO UNCOMMENT the line:
# MAKE SURE TO UNCOMMENT the line:
#     COMMON_SRC += uart.c
# so it will make your code!

# do these in order.
PROGS += output-test-3-emit.c
PROGS += output-test-1-hello-disable.c  
PROGS += output-test-0-hello.c  

# swap these if your bootloader doesn't work.
BOOTLOADER = my-install
BOOTLOADER = pi-install

# uncomment to build yours
# COMMON_SRC += uart.c

# we use a custom cstart
COMMON_SRC += cstart.c 

# set to 0 if you don't want to run.
RUN=1

L := $(CS140E_2026_PATH)/libpi/
STAFF_OBJS := $(L)/staff-objs/staff-sw-uart.o


# if doing checkoff use both uart and cstart.
ifdef CHECKOFF
    COMMON_SRC = uart.c cstart.c
    PROGS= $(wildcard output-test*.c)
endif

include $(CS140E_2026_PATH)/libpi/mk/Makefile.robust-v2

checkoff:
	make -C ./ CHECKOFF=1 check
	@echo "checking that not using staff uart: if you see output is an error:"
	arm-none-eabi-nm $(BUILD_DIR)/*.elf  | grep staff_uart
