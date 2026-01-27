// engler, cs140e: driver for "bootloader" for an r/pi connected via 
// a tty-USB device.
//
// most of it is argument parsing.
//
// Unless you know what you are doing:
//              DO NOT MODIFY THIS CODE!
//              DO NOT MODIFY THIS CODE!
//              DO NOT MODIFY THIS CODE!
//              DO NOT MODIFY THIS CODE!
//              DO NOT MODIFY THIS CODE!
//
// You shouldn't have to modify any code in this file.  Though, if you find
// a bug or improve it, let us know!
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>

#include "libunix.h"
#include "put-code.h"

static char *progname = 0;

static void usage(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    output("\nusage: %s [--trace-all] [--trace-control] [--baud <rate>] [--addr <addr>] ([device] | [--last] | [--first] [--device <device>]) <pi-program>\n", progname);
    output("    pi-program = has a '.bin' suffix\n");
    output("    specify a device using any method:\n");
    output("        <device>: has a '/dev' prefix\n");
    output("       --last: gets the last serial device mounted\n");
    output("        --first: gets the first serial device mounted\n");
    output("        --device <device>: manually specify <device>\n");
    output("    --baud <baud_rate>: manually specify baud_rate\n");
    output("    --addr <addr>: set load/jump address (e.g., 0x8000)\n");
    output("    --trace-all: trace all put/get between rpi and unix side\n");
    output("    --trace-control: trace only control [no data] messages\n");
    exit(1);
}

static unsigned long parse_ul_or_die(const char *s, const char *flag) {
    errno = 0;
    char *end = 0;
    unsigned long v = strtoul(s, &end, 0);
    if(end == s || *end != '\0')
        usage("%s: invalid number: <%s>\n", flag, s);
    if(errno == ERANGE || v > UINT_MAX)
        usage("%s: number out of range: <%s>\n", flag, s);
    return v;
}

static unsigned parse_baud_or_die(const char *s) {
    if(s[0] == 'B')
        s++;
    unsigned long rate = parse_ul_or_die(s, "--baud");
    switch(rate) {
    case 9600: return B9600;
    case 115200: return B115200;
    case 230400: return B230400;
    case 460800: return B460800;
#ifdef B576000
    case 576000: return B576000;
#endif
#ifdef B921600
    case 921600: return B921600;
#endif
#ifdef B1000000
    case 1000000: return B1000000;
#endif
    default:
        usage("--baud: unsupported rate <%s> (try 115200)\n", s);
        return B115200;
    }
}

int main(int argc, char *argv[]) { 
    char *dev_name = 0;
    char *pi_prog = 0;

    // used to pass the file descriptor to another program.
    char **exec_argv = 0;

    // a good extension challenge: tune timeout and baud rate transmission
    //
    // on linux, baud rates are defined in:
    //  /usr/include/asm-generic/termbits.h
    //
    // when I tried with sw-uart, these all worked:
    //      B9600
    //      B115200
    //      B230400
    //      B460800
    //      B576000
    // can almost do: just a weird start character.
    //      B1000000
    // all garbage?
    //      B921600
    unsigned baud_rate = B115200;

    // by default is 0x8000
    unsigned boot_addr = ARMBASE;

    // we do manual option parsing to make things a bit more obvious.
    // you might rewrite using getopt().
    progname = argv[0];
    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--trace-control") == 0)  {
            trace_p = TRACE_CONTROL_ONLY;
        } else if(strcmp(argv[i], "--trace-all") == 0)  {
            trace_p = TRACE_ALL;
        } else if(strcmp(argv[i], "--last") == 0)  {
            dev_name = find_ttyusb_last();
        } else if(strcmp(argv[i], "--first") == 0)  {
            dev_name = find_ttyusb_first();
        // we assume: anything that begins with /dev/ is a device.
        } else if(prefix_cmp(argv[i], "/dev/")) {
            dev_name = argv[i];
        // we assume: anything that ends in .bin is a pi-program.
        } else if(suffix_cmp(argv[i], ".bin")) {
            pi_prog = argv[i];
        } else if(strcmp(argv[i], "--baud") == 0) {
            i++;
            if(!argv[i])
                usage("missing argument to --baud\n");
            baud_rate = parse_baud_or_die(argv[i]);
        } else if(strcmp(argv[i], "--addr") == 0) {
            i++;
            if(!argv[i])
                usage("missing argument to --addr\n");
            boot_addr = parse_ul_or_die(argv[i], "--addr");
            if(boot_addr == 0 || boot_addr >= (512u * 1024u * 1024u))
                usage("--addr has invalid address: %x\n", boot_addr);
        } else if(strcmp(argv[i], "--exec") == 0) {
            i++;
            if(!argv[i])
                usage("missing argument to --exec\n");
            exec_argv = &argv[i];
            break;
        } else if(strcmp(argv[i], "--device") == 0) {
            i++;
            if(!argv[i])
                usage("missing argument to --device\n");
            dev_name = argv[i];
        } else {
            usage("unexpected argument=<%s>\n", argv[i]);
        }
    }
    if(!pi_prog)
        usage("no pi program\n");

    // 1. get the name of the ttyUSB.
    if(!dev_name) {
        dev_name = find_ttyusb_last();
        if(!dev_name)
            panic("didn't find a device\n");
    }
    debug_output("done with options: dev name=<%s>, pi-prog=<%s>, trace=%d\n", 
            dev_name, pi_prog, trace_p);

    if(exec_argv)
        argv_print("BOOT: --exec argv:", exec_argv);

    // 2. open the ttyUSB in 115200, 8n1 mode
    int tty = open_tty(dev_name);
    if(tty < 0)
        panic("can't open tty <%s>\n", dev_name);

    // timeout is in seconds. tuning this can speed up checking.
    //
    // if your my-install isn't resetting when used during checking,
    // it's likely due to this timeout being too small.
    double timeout_secs = 1.0;
    int fd = set_tty_to_8n1(tty, baud_rate, timeout_secs);
    if(fd < 0)
        panic("could not set tty: <%s>\n", dev_name);

    // 3. read in program [probably should just make a <file_size>
    //    call and then shard out the pieces].
	unsigned nbytes;
    uint8_t *code = read_file(&nbytes, pi_prog);

    // 4. let's send it!
	debug_output("%s: tty-usb=<%s> program=<%s>: about to boot\n", 
                progname, dev_name, pi_prog);
    simple_boot(fd, boot_addr, code, nbytes);

    // 5. echo output from pi
    if(!exec_argv)
        pi_echo(0, fd, dev_name);
    else {
        todo("not handling exec_argv");
        handoff_to(fd, TRACE_FD, exec_argv);
    }
	return 0;
}
