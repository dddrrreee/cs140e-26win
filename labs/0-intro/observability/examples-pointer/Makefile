# good breakdown: http://cs107e.github.io/guides/make/
ODFLAG = -d 
ODFLAG = -d  -S 
ODFLAG = -d 

# CFLAGS = -O3 -g -Wall -Werror -std=gnu99
CFLAGS = -O3 -g -std=gnu99
# -fno-strict-aliasing

PROGS := $(wildcard *.c)
PROGS := $(PROGS:.c=.o)

all : $(PROGS)

%.o: %.c
	arm-none-eabi-gcc $(CFLAGS) -c $^ -o $@
	arm-none-eabi-objdump $(ODFLAG) $@ >  $@.dis
	arm-none-eabi-objdump $(ODFLAG) -S $@ >  $@.dis.annot
	cat $@.dis

clean:
	rm -f  *.dis *.o *~ *.s
