########################################################################
# Testing makefile for 1-fake-pi
#
# Usage:
#   - Uncomment TEST_SRC lines below to select which tests to run
#   - make run       : run all tests in TEST_SRC
#   - make emit      : generate .out files for all tests
#   - make check     : check tests against their .out files
#   - make cksum     : compute checksums
#   - make checkoff  : generate checkoff checksum
#
# Test progression (uncomment as you go):
#   1. Start with single easy test: 0-gpio-write-17.c
#   2. All 0-*.c tests (easy)
#   3. All 1-*.c tests (harder)
#   4. act-*.c tests (need to add addresses to fake-pi.c)

# Select which tests to run (last writer wins)
TEST_SRC += 0-gpio-write-17.c
# TEST_SRC := $(wildcard ./[0]-*.c)
# TEST_SRC += $(wildcard ./[1]-*.c)
# TEST_SRC += $(wildcard ./[2]-*.c) 
# TEST_SRC += $(wildcard ./act-*.c)
# TEST_SRC += $(wildcard ./prog-*.c) 

# Checkoff uses all tests
CHKOFF_SRC := $(wildcard ./[012]-*.c) $(wildcard ./prog-*.c) $(wildcard ./act-*.c)
CHKOFF_OUT := $(CHKOFF_SRC:.c=.out)

########################################################################

MAKEFLAGS += --no-print-directory

LIBPI_FAKE = ../libpi-fake.a
FAKE_PROGS := $(TEST_SRC:.c=.fake)

# Dependencies: Makefile changes should trigger rebuild
DEPS := $(filter-out %.d, $(MAKEFILE_LIST))

INC = -I../ -I. -I$(CS140E_2026_PATH)/libpi/include -I$(CS140E_2026_PATH)/libpi/libc
CFLAGS = $(INC) -O -Wall -Wno-unused-function -Wno-unused-variable

MK_CHK := ./Makefile.test

all: libpi-fake-check $(FAKE_PROGS)

# Use separate PHONY target to check library without marking the 
# library itself as PHONY. This ensures the sub-make always runs 
# to check if rebuild is needed, but <.fake> programs only rebuild 
# if the library file's timestamp actually changes
.PHONY: libpi-fake-check
libpi-fake-check:
	@make -C .. libpi-fake.a

%.fake: %.c $(LIBPI_FAKE) $(DEPS)
	$(CC) $(CFLAGS) $< -o $@ $(LIBPI_FAKE)

# Test commands
emit: libpi-fake-check $(FAKE_PROGS)
	@make -f $(MK_CHK) emitall "TESTS=$(FAKE_PROGS)"
run: libpi-fake-check $(FAKE_PROGS)
	@make -f $(MK_CHK) runall "TESTS=$(FAKE_PROGS)"
check: libpi-fake-check $(FAKE_PROGS)
	@make -f $(MK_CHK) checkall "TESTS=$(FAKE_PROGS)"
cksum: libpi-fake-check $(FAKE_PROGS)
	@make -f $(MK_CHK) cksumall "TESTS=$(FAKE_PROGS)"
ref: libpi-fake-check $(FAKE_PROGS)
	@make -f $(MK_CHK) refall "TESTS=$(FAKE_PROGS)"

# Generate checkoff checksum
checkoff:
	@make -C ./ emit
	@echo "--- Checksums of all output files ---"
	@ls $(CHKOFF_OUT) | sort | xargs cksum
	@echo ""
	@echo "Total files: $$(ls $(CHKOFF_OUT) | wc -l)"
	@echo ""
	@echo "=== USE THIS VALUE FOR CHECKOFF ==="
	@ls $(CHKOFF_OUT) | sort | xargs cksum | cksum

cleanall: clean
	rm -f *.out *.raw

clean:
	rm -f *.fake *~ *.o Makefile.bak

.PHONY: all clean cleanall emit run check cksum ref checkoff libpi-fake-check
