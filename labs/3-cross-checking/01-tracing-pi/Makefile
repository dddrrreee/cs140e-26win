# Simple Makefile for 01-tracing-pi: demonstrates ld <--wrap> flag
#
# This uses one of the standard built-in makefiles from <libpi/mk/>.
# You just need to define a few variables and include the template.

# The program to build (must have a notmain)
PROGS := trace-foo.c

# Additional source files to compile and link
COMMON_SRC := foo.c

# Flags for the linker: <--wrap=foo> tells ld to intercept calls to <foo>
LDFLAGS = --wrap=foo

# Bootloader to use for uploading to pi
BOOTLOADER = pi-install

# Set <RUN=1> to automatically run the .bin files after building
RUN=1

# Set this if you need to specify the device for your pi
# TTYUSB =

# Include the standard template which defines all the build rules
include $(CS140E_2026_PATH)/libpi/mk/Makefile.template-fixed
