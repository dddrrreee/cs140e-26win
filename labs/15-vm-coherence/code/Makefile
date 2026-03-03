# You'll modify these two files.  see below for what to do
# when done.
COMMON_SRC += your-mmu-asm.S
COMMON_SRC += mmu.c

# all the tests should pass out of the box.
PROGS := tests/1-test-basic-tutorial.c
PROGS += tests/1-test-one-addr.c
PROGS += tests/1-test-two-addr.c
PROGS += tests/2-test-lookup.c
PROGS += tests/2-test-procmap.c

# New test!
#  - uses PMU performance counters to check if caches 
#    were flushed, etc.
PROGS += tests/4-test-vm-cache-mgmt.c

##########################################################
# WHEN DONE do two things.
#
# 1. get rid of <orig-pinned-vm.c> and use your lab 13.
#    comment out:
COMMON_SRC += orig-pinned-vm.c
#    uncomment, copy your pinned-vm.c over.
# COMMON_SRC += pinned-vm.c

# 2. comment these out!  all done.
STAFF_OBJS += staff-pinned-vm.o
STAFF_OBJS += staff-mmu-asm.o  
STAFF_OBJS += staff-mmu.o  
##########################################################

# next week you'll get rid of all of these
O = $(CS140E_2026_PATH)/libpi
STAFF_OBJS += $(O)/staff-objs/staff-full-except.o
STAFF_OBJS += $(O)/staff-objs/staff-full-except-asm.o
STAFF_OBJS += $(O)/staff-objs/staff-switchto-asm.o
STAFF_OBJS += $(O)/staff-objs/staff-kmalloc.o

BOOTLOADER = my-install
TTYUSB = 
RUN = 1
include $(CS140E_2026_PATH)/libpi/mk/Makefile.robust-v3

clean::
	make -C tests clean
