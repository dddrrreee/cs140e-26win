# our hacked kernel.
PROGS := pix.c

##################################################################
#
# concat as many of these as you want to <USER_PROG>: will run 
# them all and check the hashes.
#

# they are compiled from <user-progs> but since people have
# different compilers we provide prebuilt binaries.

U := user-progs-prebuilt/

USER_PROG = $(U)/1-fork.bin
USER_PROG = $(U)/0-hello.bin
USER_PROG = $(U)/0-printk-hello.bin
USER_PROG = $(U)/1-fork-waitpid.bin
USER_PROG2 = $(U)/1-fork-waitpid.bin

USER_PROG = $(U)/1-fork.bin
USER_PROG += $(U)/0-hello.bin $(U)/0-hello.bin
USER_PROG += $(U)/1-fork-waitpid.bin
USER_PROG += $(U)/0-printk-hello.bin
USER_PROG += $(U)/0-printk-hello.bin
USER_PROG += $(U)/0-printk-hello.bin
USER_PROG += $(U)/0-printk-hello.bin
USER_PROG += $(U)/0-printk-hello.bin
USER_PROG += $(U)/0-printk-hello.bin

# at this point will run a ton of programs.


# just run hello world as a start.  when this works,
# comment it out so all the others run!
# USER_PROG = $(U)/0-hello.bin

O := $(CS140E_2026_PATH)/libpi/
STAFF_OBJS += $(O)/staff-objs/staff-kmalloc.o

L := $(CS140E_2026_PATH)/labs/

#####################################################
# lab 13: pinned: just swap yours in

PINNED := $(CS140E_2026_PATH)/labs/13-pinned-vm/code/
STAFF_OBJS +=  $(PINNED)/staff-pinned-vm.o
STAFF_OBJS +=  $(PINNED)/staff-mmu.o
STAFF_OBJS +=  $(PINNED)/staff-mmu-asm.o

MMU := $(CS140E_2026_PATH)/labs/15-vm-coherence/code/
INC += -I$(MMU)

#####################################################
# lab 11: debug: just swap yours in

DEBUG := $(CS140E_2026_PATH)/labs/11-debug-hw/code
INC += -I$(DEBUG)
# swap yours in and ours out.
# COMMON_SRC += $(DEBUG)/breakpoint.c
STAFF_OBJS += $(O)/staff-objs/staff-breakpoint.o

#####################################################
# lab 12: preemtive: just swap yours in

SWITCHTO := $(CS140E_2026_PATH)/labs/12-preemptive/2-save-restore/
INC += -I$(SWITCHTO)

# swap yours in and ours out.
# COMMON_SRC += $(SWITCHTO)/switchto-asm.S
STAFF_OBJS += $(O)/staff-objs/staff-switchto-asm.o

# you'll have to write this: it's just the exception vectors
# with the full registers saved.
# COMMON_SRC += forward-except.S
STAFF_OBJS += staff-forward-except.o


BOOTLOADER=my-install
RUN = 0
CAN_EMIT=1
EXCLUDE ?= grep -v simple_boot
include $(CS140E_2026_PATH)/libpi/mk/Makefile.robust-v3

all::  tags init.bin

# we just concat all binaries into one big binary and load it.
init.bin:  pix.bin 
	make -C user-progs
	cat pix.bin $(USER_PROG) > init.bin
	my-install init.bin

clean::
	rm -f init.bin

.PHONEY: update

update:
	cp objs/l1/l2/l3/staff-forward-except.o ../code
