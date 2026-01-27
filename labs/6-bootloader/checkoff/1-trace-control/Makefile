# start with this one if you have trouble.
TESTS = hello.bin
TESTS = $(wildcard ./*.bin)

# should not need this!
# TTYUSB = 

BOOTLOADER = my-install --trace-control
RUN=1

# we only look at this output
GREP_STR := "TRACE:"
# so we don't lose simple_boot
EXCLUDE := cat

include $(CS140E_2026_PATH)/libpi/mk/Makefile.check-v3
