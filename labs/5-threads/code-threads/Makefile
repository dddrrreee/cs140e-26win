# first step:
#  - do "make check"
#  - all the tests should pass.
#  - then start implementing your code.
#  
# todays lab, implement:
#   - <rpi-thread-asm.S>
#   - <rpi-thread.c>
#
# then set 
#    PROGS = $(wildcard [234567]-test*.c)
#
# - "make check" should pass.
# - "make checkoff" should give the same checksum as in the README.md


# set this to 0 if you want the programs to NOT run automatically.
RUN=1

S := $(CS140E_2026_PATH)/libpi/staff-objs/
STAFF_OBJS += $(S)/staff-kmalloc.o

# switch these when implemented.
# COMMON_SRC += rpi-thread-asm.S 
STAFF_OBJS += $(S)/staff-rpi-thread-asm.o

# switch these when implemented.
# COMMON_SRC += rpi-thread.c
STAFF_OBJS += $(S)/staff-rpi-thread.o

# all the tests.  

# the different tests: start with 1, then 2, then..
PROGS := $(wildcard [1234567]-test*.c)
PROGS := $(wildcard 7-test*.c)
PROGS := $(wildcard 5-test*.c)
PROGS := $(wildcard 4-test*.c)
PROGS := $(wildcard 3-test*.c)
PROGS := $(wildcard 2-test*.c)
PROGS := $(wildcard 1-test*.c)

# sanity check that redzones detect errors: use this!
PROGS := $(wildcard 0-test*.c)

######################################################################
# should not have to modify below.
#

CAN_EMIT=0
ALLPROG := $(wildcard [34567]-test*.c)


# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = pi-install

# checkoff skips 1 since behavior changes
ifeq ($(MAKECMDGOALS),checkoff)
    PROGS = $(wildcard [234567]-test*.c)
    USE_STAFF = 0
endif

include $(CS140E_2026_PATH)/libpi/mk/Makefile.robust-v2


# GREP_STR := 'TRACE:\|ERROR:\|PANIC:'

checkoff: check
	@echo "--------------------------------------------------------";
	@echo "Environment:";
	@echo "   STAFF_OBJS [should just have staff-kmalloc.o] =<$(STAFF_OBJS)>";
	@echo "   COMMON_SRC [should have both .o] =<$(COMMON_SRC)>";
	@echo -n "   total tests=";
	@ls $(PROGS:.c=.out) | wc -l
	@echo -n "   checksum of cksum: ";
	cksum $(PROGS:.c=.out) | sort -n | cksum

cksum:
	ls $(ALLPROG:.c=.out) | wc -l
	cksum $(ALLPROG:.c=.out) | sort -n | cksum

.PHONEY: checkoff
