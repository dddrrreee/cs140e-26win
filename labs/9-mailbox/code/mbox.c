#include "rpi.h"
#include "mbox.h"

// dump out the entire messaage.  useful for debug.
void msg_dump(const char *msg, volatile uint32_t *u, unsigned nwords) {
    printk("%s\n", msg);
    for(int i = 0; i < nwords; i++)
        output("u[%d]=%x\n", i,u[i]);
}

void mailbox_msg(volatile uint32_t* msg, uint32_t tag, volatile uint32_t* values, uint32_t value_words, int debug) {
    // make sure aligned
    assert((unsigned)msg%16 == 0);
    
    uint32_t total_msg_bytes = (6 + value_words) * 4;
    // memset(msg, 0, total_msg_bytes);

    /* Header */
    msg[0] = total_msg_bytes;         // total size in bytes.
    msg[1] = 0;           // sender: always 0.

    /* Tags */
    msg[2] = tag;
    msg[3] = value_words*4; 
    msg[4] = 0;

    for (int i = 0; i < value_words; i++) {
        msg[5 + i] = values[i];
    }

    /* End tag */
    msg[5 + value_words] = 0;

    if (debug) {
        // if you want to debug.
        output("got:\n");
        for(int i = 0; i < 8; i++)
            output("msg[%d]=%x\n", i, msg[i]);
    }

    // send and receive message
    mbox_send(MBOX_CH, msg);

    if (debug) {
        // if you want to debug.
        output("got:\n");
        for(int i = 0; i < 8; i++)
            output("msg[%d]=%x\n", i, msg[i]);
    }

    // should have value for success: 1<<31
    if(msg[1] != 0x80000000)
		panic("invalid response: got %x\n", msg[1]);

    // high bit should be set and reply size
    assert(msg[4] == ((1<<31) | value_words*4));
}

#define AAA 1
#if (AAA == 1)

uint64_t rpi_get_serialnum(void) {
    volatile uint32_t msg[8] __attribute__((aligned(16)));
    volatile uint32_t values[2]  __attribute__((aligned(16)));

    mailbox_msg(msg, 0x00010004, values, 2, 1);

    assert(msg[6] == 0);
    return msg[5];
}

// msg[0]=0x20
// msg[1]=0x0
// msg[2]=0x10004
// msg[3]=0x8
// msg[4]=0x0
// msg[5]=0x0
// msg[6]=0x0
// msg[7]=0x0

#endif

/*
  This is given.

  Get board serial
    Tag: 0x00010004
    Request: Length: 0
    Response: Length: 8
    Value: u64: board serial
*/
# if (AAA == 0)
uint64_t rpi_get_serialnum(void) {
    // 16-byte aligned 32-bit array
    volatile uint32_t msg[8] __attribute__((aligned(16)));

    // make sure aligned
    assert((unsigned)msg%16 == 0);

    msg[0] = 8*4;         // total size in bytes.
    msg[1] = 0;           // sender: always 0.
    msg[2] = 0x00010004;  // serial tag
    msg[3] = 8;           // total bytes avail for reply
    msg[4] = 0;           // request code [0].
    msg[5] = 0;           // space for 1st word of reply 
    msg[6] = 0;           // space for 2nd word of reply 
    msg[7] = 0;   // end tag

#if 1
    // if you want to debug.
    output("got:\n");
    for(int i = 0; i < 8; i++)
        output("msg[%d]=%x\n", i, msg[i]);

#endif
        
    // send and receive message
    mbox_send(MBOX_CH, msg);

#if 1
    // if you want to debug.
    output("got:\n");
    for(int i = 0; i < 8; i++)
        output("msg[%d]=%x\n", i, msg[i]);

#endif
        
    /*
    msg[0]=0x20         // size again
    msg[1]=0x80000000   // Successful
    msg[2]=0x10004      // Tag
    msg[3]=0x8          // Reply bytes
    msg[4]=0x80000008   // Request code (0x8... means response, ...8 is value length in bytes)
    msg[5]=0xa8249570   // Response
    msg[6]=0x0
    msg[7]=0x0
    */

    // should have value for success: 1<<31
    if(msg[1] != 0x80000000)
		panic("invalid response: got %x\n", msg[1]);

    // high bit should be set and reply size
    assert(msg[4] == ((1<<31) | 8));

    // for me the upper 32 bits were never non-zero.  
    // not sure if always true?
    assert(msg[6] == 0);
    return msg[5];
}

#endif
uint32_t rpi_get_memsize(void) {

    // 16-byte aligned 32-bit array
    volatile uint32_t msg[8] __attribute__((aligned(16)));

    // make sure aligned
    assert((unsigned)msg%16 == 0);

    msg[0] = 8*4;         // total size in bytes.
    msg[1] = 0;           // sender: always 0.
    msg[2] = 0x00010005;  // serial tag
    msg[3] = 8;           // total bytes avail for reply
    msg[4] = 0;           // request code [0].
    msg[5] = 0;           // space for 1st word of reply 
    msg[6] = 0;           // space for 2nd word of reply 
    msg[7] = 0;   // end tag

    // send and receive message
    mbox_send(MBOX_CH, msg);
    // todo("get the pi's physical memory size");

#if 1
    // if you want to debug.
    output("got:\n");
    for(int i = 0; i < 8; i++)
        output("msg[%d]=%x\n", i, msg[i]);

#endif

    // should have value for success: 1<<31
    if(msg[1] != 0x80000000)
		panic("invalid response: got %x\n", msg[1]);

    // high bit should be set and reply size
    assert(msg[4] == ((1<<31) | 8));

    // for me the upper 32 bits were never non-zero.  
    // not sure if always true?
    // assert(msg[6] == 0);
    return msg[6];

}


uint32_t rpi_get_model(void) {
    // todo("get the pi's model number");
    // 16-byte aligned 32-bit array
    volatile uint32_t msg[7] __attribute__((aligned(16)));
    assert((unsigned)msg%16 == 0);

    msg[0] = 8*4;         // total size in bytes.
    msg[1] = 0;           // sender: always 0.
    msg[2] = 0x00010001;  // serial tag
    msg[3] = 4;           // total bytes avail for reply
    msg[4] = 0;           // request code [0].
    msg[5] = 0;           // space for 1st word of reply (model number)
    msg[6] = 0;   // end tag

    // send and receive message
    mbox_send(MBOX_CH, msg);
    // todo("get the pi's physical memory size");

#if 1
    // if you want to debug.
    output("got:\n");
    for(int i = 0; i < 7; i++)
        output("msg[%d]=%x\n", i, msg[i]);

#endif

    // should have value for success: 1<<31
    if(msg[1] != 0x80000000)
		panic("invalid response: got %x\n", msg[1]);

    // high bit should be set and reply size
    assert(msg[4] == ((1<<31) | 4));

    // for me the upper 32 bits were never non-zero.  
    // not sure if always true?
    assert(msg[6] == 0);
    return msg[5];
}

// https://www.raspberrypi-spy.co.uk/2012/09/checking-your-raspberry-pi-board-version/
uint32_t rpi_get_revision(void) {
    // 16-byte aligned 32-bit array
    volatile uint32_t msg[7] __attribute__((aligned(16)));
    assert((unsigned)msg%16 == 0);

    msg[0] = 8*4;         // total size in bytes.
    msg[1] = 0;           // sender: always 0.
    msg[2] = 0x00010002;  // serial tag
    msg[3] = 4;           // total bytes avail for reply
    msg[4] = 0;           // request code [0].
    msg[5] = 0;           // space for 1st word of reply 
    msg[6] = 0;   // end tag

    // send and receive message
    mbox_send(MBOX_CH, msg);
    // todo("get the pi's physical memory size");

#if 1
    // if you want to debug.
    output("got:\n");
    for(int i = 0; i < 7; i++)
        output("msg[%d]=%x\n", i, msg[i]);

#endif

    // should have value for success: 1<<31
    if(msg[1] != 0x80000000)
		panic("invalid response: got %x\n", msg[1]);

    // high bit should be set and reply size
    assert(msg[4] == ((1<<31) | 4));

    // for me the upper 32 bits were never non-zero.  
    // not sure if always true?
    assert(msg[6] == 0);
    return msg[5];
}

uint32_t rpi_temp_get(void) {

    // 16-byte aligned 32-bit array
    volatile uint32_t msg[8] __attribute__((aligned(16)));
    assert((unsigned)msg%16 == 0);

    msg[0] = 8*4;         // total size in bytes.
    msg[1] = 0;           // sender: always 0.
    msg[2] = 0x00030006;  // serial tag
    msg[3] = 8;           // total bytes avail for reply
    msg[4] = 0;           // request code [0].
    msg[5] = 0;           // space for 1st word of reply
    msg[6] = 0;           // space for 2nd word of reply
    msg[7] = 0;   // end tag

    // send and receive message
    mbox_send(MBOX_CH, msg);

#if 1
    // if you want to debug.
    output("got:\n");
    for(int i = 0; i < 8; i++)
        output("msg[%d]=%x\n", i, msg[i]);

#endif

    // should have value for success: 1<<31
    if(msg[1] != 0x80000000)
		panic("invalid response: got %x\n", msg[1]);

    // high bit should be set and reply size
    assert(msg[4] == ((1<<31) | 8));

    // for me the upper 32 bits were never non-zero.  
    // not sure if always true?
    // assert(msg[6] == 0);
    return msg[6];
}

void rpi_allocate_memory(uint32_t bytes) {

    // 16-byte aligned 32-bit array
    volatile uint32_t msg[9] __attribute__((aligned(16)));
    assert((unsigned)msg%16 == 0);

    msg[0] = 9*4;         // total size in bytes.
    msg[1] = 0;           // sender: always 0.

    msg[2] = 0x0003000C;  // serial tag
    msg[3] = 12;  // Value buffer size
    msg[4] = 0;           // request [0]

    msg[5] = bytes;           // size
    msg[6] = 4;           // alignment
    msg[7] = 0;           // flags

    msg[8] = 0;   // end tag

    // send and receive message
    mbox_send(MBOX_CH, msg);

#if 1
    // if you want to debug.
    output("got:\n");
    for(int i = 0; i < 9; i++)
        output("msg[%d]=%x\n", i, msg[i]);

#endif

    // high bit should be set and reply size
    assert(msg[4] == ((1<<31) | 4));
}

uint32_t rpi_clock_curhz_get(uint32_t clock) {

    // 16-byte aligned 32-bit array
    volatile uint32_t msg[36] __attribute__((aligned(16)));
    assert((unsigned)msg%16 == 0);

    msg[0] = 36*4;         // total size in bytes.
    msg[1] = 0;           // sender: always 0.

    msg[2] = 0x0010007;  // serial tag
    msg[3] = 15*4;  // Value buffer size (15 clocks)
    msg[4] = 0;           // request [0]

    msg[35] = 0;   // end tag

    // send and receive message
    mbox_send(MBOX_CH, msg);

#if 1
    // if you want to debug.
    output("got:\n");
    for(int i = 0; i < 36; i++)
        output("msg[%d]=%x\n", i, msg[i]);

#endif

    // high bit should be set and reply size
    assert(msg[4] == ((1<<31) | 60));
    return 0;
}