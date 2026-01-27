PROGS := main.c
COMMON_SRC := 

# we have to override start --- look at it!
OUR_START := boot-start.S

# we can't bootload the bootloader.
RUN=0

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = pi-install

include $(CS140E_2026_PATH)/libpi/mk/Makefile.robust-v2

all:: kernel.img

clean::
	rm -f $(START) kernel.img

kernel.img: main.bin
	cp main.bin kernel.img
