# Makefile for 02-tracing-linux: demonstrates `--wrap` on Linux
#
# This is the same mechanism as 01-tracing-pi but runs as a regular
# Linux program. Useful for testing wrapping logic with fast cycles.

# The program to build
NAME = trace-foo

# Source files (compiled to .o and linked)
SRC = trace-foo.o foo.o

# Header files (dependencies)
HDRS = foo.h

# Tell `ld` to wrap `foo`
# `-Wl,` passes flags to the linker (ld) through gcc
# EDIT: Added angle brackets for consistency
WRAP_FLAGS = -Wl,--wrap=foo

# Compiler flags
CFLAGS = -O -Wall -Wno-unused-function -Wno-unused-variable $(WRAP_FLAGS)

# Default target: build and run
all: $(NAME) run

# Link the program
$(NAME): $(SRC) $(HDRS)
	$(CC) $(CFLAGS) $(SRC) -o $(NAME)

# Run the program and show expected output
run:
	@echo "if --wrap worked: should see calls to both 'wrap_foo' and 'real_foo'";
	./$(NAME)

# Clean up generated files
clean:
	rm -rf $(NAME) *~ *.o Makefile.bak

.PHONY: all clean run
