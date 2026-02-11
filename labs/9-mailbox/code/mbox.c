#include "rpi.h"
#include "mbox.h"

// dump out the entire messaage.  useful for debug.
void msg_dump(const char *msg, volatile uint32_t *u, unsigned nwords) {
    printk("%s\n", msg);
    for(int i = 0; i < nwords; i++)
        output("u[%d]=%x\n", i,u[i]);
}

static void mailbox_msg(uint32_t tag, volatile uint32_t* values, uint32_t request_words, uint32_t response_words, int debug) {
    uint32_t words = request_words < response_words ? response_words : request_words;
    uint32_t total_msg_words = 6 + words;

    volatile uint32_t msg[total_msg_words] __attribute__((aligned(16)));
    // make sure aligned
    assert((unsigned)msg%16 == 0);

    /* Header */
    msg[0] = total_msg_words*4;         // total size in bytes.
    msg[1] = 0;           // sender: always 0.

    /* Tags */
    msg[2] = tag;
    msg[3] = words*4; 
    msg[4] = 0;

    for (int i = 0; i < words; i++) {
        msg[5 + i] = values[i];
    }

    /* End tag */
    msg[5 + words] = 0;

    if (debug) {
        // if you want to debug.
        output("Before:\n");
        for(int i = 0; i < total_msg_words; i++)
            output("msg[%d]=%x\n", i, msg[i]);
    }

    // send and receive message
    mbox_send(MBOX_CH, msg);

    if (debug) {
        // if you want to debug.
        output("After:\n");
        for(int i = 0; i < total_msg_words; i++)
            output("msg[%d]=%x\n", i, msg[i]);
    }

    // should have value for success: 1<<31
    if(msg[1] != 0x80000000)
		panic("invalid response: got %x\n", msg[1]);

    // high bit should be set and reply size
    assert(msg[4] == ((1<<31) | response_words*4));

    for (int i = 0; i < response_words; i++) {
        values[i] = msg[5 + i];
    }
}

/*
  This is given.

  Get board serial
    Tag: 0x00010004
    Request: Length: 0
    Response: Length: 8
    Value: u64: board serial
*/
uint64_t rpi_get_serialnum(void) {
    volatile uint32_t values[2]  __attribute__((aligned(16)));
    int debug = 0;
    mailbox_msg(0x00010004, values, 2, 2, debug);

    assert(values[1] == 0);
    return values[0];
}

uint32_t rpi_get_memsize(void) {
    volatile uint32_t values[2]  __attribute__((aligned(16)));
    int debug = 0;
    mailbox_msg(0x00010005, values, 0, 2, debug);
    return values[1];
}

uint32_t rpi_get_model(void) {
    volatile uint32_t values[1]  __attribute__((aligned(16)));
    int debug = 0;
    mailbox_msg(0x00010001, values, 0, 1, debug);
    return values[0];
}

// https://www.raspberrypi-spy.co.uk/2012/09/checking-your-raspberry-pi-board-version/
uint32_t rpi_get_revision(void) {
    volatile uint32_t values[1]  __attribute__((aligned(16)));
    int debug = 0;
    mailbox_msg(0x00010002, values, 0, 1, debug);
    return values[0];
}

uint32_t rpi_temp_get(void) {
    volatile uint32_t values[2]  __attribute__((aligned(16)));
    int debug = 0;
    mailbox_msg(0x00030006, values, 1, 2, debug);
    return values[1];
}

uint32_t rpi_allocate_memory(uint32_t bytes) { // ** DOES THIS WORK?
    volatile uint32_t values[3]  __attribute__((aligned(16)));
    values[0] = bytes;
    values[1] = 4; // Alignment
    values[2] = 0; // Flags

    int debug = 0;
    mailbox_msg(0x0003000C, values, 3, 1, debug);

    return values[0];
}

// uint32_t rpi_clock_curhz_get(uint32_t clock) {
//     volatile uint32_t values[30]  __attribute__((aligned(16)));
//     int debug = 1;
//     mailbox_msg(0x00010007, values, 0, 30, debug);

//     // First byte is clock ID then actual value
//     return values[2*clock];
// }

uint32_t rpi_clock_curhz_get(uint32_t clock) {
    volatile uint32_t values[2]  __attribute__((aligned(16)));
    values[0] = clock;
    int debug = 0;
    mailbox_msg(0x00030002, values, 1, 2, debug);

    assert(values[0] == clock);
    return values[1];
}

uint32_t rpi_clock_maxhz_get(uint32_t clock) {
    volatile uint32_t values[2]  __attribute__((aligned(16)));
    values[0] = clock;
    int debug = 0;
    mailbox_msg(0x00030004, values, 1, 2, debug);

    assert(values[0] == clock);
    return values[1];
}

uint32_t rpi_clock_minhz_get(uint32_t clock) {
    volatile uint32_t values[2]  __attribute__((aligned(16)));
    values[0] = clock;
    int debug = 0;
    mailbox_msg(0x00030007, values, 1, 2, debug);

    assert(values[0] == clock);
    return values[1];
}

uint32_t rpi_clock_realhz_get(uint32_t clock) {
    volatile uint32_t values[2]  __attribute__((aligned(16)));
    values[0] = clock;
    int debug = 0;
    mailbox_msg(0x00030047, values, 1, 2, debug);

    assert(values[0] == clock);
    return values[1];
}

uint32_t rpi_clock_hz_set(uint32_t clock, uint32_t hz) {
    volatile uint32_t values[3]  __attribute__((aligned(16)));
    values[0] = clock;
    values[1] = hz;
    values[2] = 0; // weird turbo thing
    int debug = 0;
    mailbox_msg(0x00038002, values, 3, 2, debug);

    assert(values[0] == clock);
    return values[1];
}

uint32_t rpi_get_voltage(uint32_t voltage_id) {
    volatile uint32_t values[2]  __attribute__((aligned(16)));
    values[0] = voltage_id;
    int debug = 0;
    mailbox_msg(0x00030003, values, 1, 2, debug);

    assert(values[0] == voltage_id);
    return values[1];
}