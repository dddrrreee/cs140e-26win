BOOTLOADER = my-install

# roughly increasing difficulty
PROGS := tests/1-test-basic-tutorial.c
PROGS += tests/1-test-one-addr.c
PROGS += tests/1-test-two-addr.c
PROGS += tests/2-test-procmap.c
PROGS += tests/3-test-cache.c
PROGS += tests/4-test-dcache-inv.c  
PROGS += tests/4-test-vm-cache-mgmt.c

# all the tests.
PROGS := $(wildcard tests/[1234]-*c)

# useful helpers: has print of page table entries, hashing etc.
COMMON_SRC += mmu-helpers.c

# this is the one file you'll modify.  then get rid of our
# staff .o's (see right below)
COMMON_SRC += pt-vm.c

#############################################################
# WHEN DONE: get rid of staff .o's doing the two steps below
# WHEN DONE: get rid of staff .o's doing the two steps below
# WHEN DONE: get rid of staff .o's doing the two steps below
# WHEN DONE: get rid of staff .o's doing the two steps below

# Step 1. comment this out and make sure tests pass.
STAFF_OBJS += staff-pt-vm.o

# Step 2. comment out our code and put in yours
#  Comment all these out (they are just clean copies of the starter 
#  code from lab 15):
COMMON_SRC += ORIG-your-mmu-asm.S
COMMON_SRC += ORIG-mmu.c

# Comment these staff .o's out. 
STAFF_OBJS += staff-mmu.o
STAFF_OBJS += staff-mmu-asm.o

# Uncommment these.  make sure tests pass.
YOUR_VM = $(CS140E_2026_PATH)/labs/15-vm-coherence/code/
# COMMON_SRC += $(YOUR_VM)/mmu.c
# COMMON_SRC += $(YOUR_VM)/your-mmu-asm.S

# if tests pass: DONE!  
# if tests pass: DONE!  
# if tests pass: DONE!  

##########################################################

O = $(CS140E_2026_PATH)/libpi
STAFF_OBJS += $(O)/staff-objs/staff-full-except.o
STAFF_OBJS += $(O)/staff-objs/staff-full-except-asm.o
STAFF_OBJS += $(O)/staff-objs/staff-switchto-asm.o
STAFF_OBJS += $(O)/staff-objs/staff-kmalloc.o

TTYUSB = 

# set RUN = 1 if you want the code to automatically run after building.
RUN = 1

GREP_STR := 'TRACE:\|HASH:\|ERROR:\|PANIC:\|SUCCESS:'
include $(CS140E_2026_PATH)/libpi/mk/Makefile.robust-v3

clean::
	make -C tests clean
