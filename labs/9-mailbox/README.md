## bcm2835 mailboxes: memory size, serial numbers and overclocking.

The upcoming labs will get more intense, so before that we'll do a quick
fun lab that has light reading and very little starter code as a small
respite --- using the GPU mailbox interface to:
 1. Get the unique serial number from your pi (this will be useful for
    the upcoming networking lab).
 2. Set and get the actual amount of physical memory you have available
    on the pi.  This is needed for Tuesday's lab and the virtual memory 
    labs coming up.
 3. Overclock the pi: you can use the mailbox to set memory speed,
    CPU speed and the BCM2835 speed.  Speed will definitely help some
    of you in final projects and for speed hacking if you take 240lx.
    You can probably spend a few days doing tricks to tweak your pi as
    fast as possible.  (At the extreme people use dry ice to cool and
    I've heard of people submersing it in mineral oil as a coolant.)

Readings:
  - [mailbox messages][mailbox-messages]
  - [valvers mailbox writeup][valvers]

The pi is a bit weird in that the VideoCore GPU controls a lot of the
action.  The firmware (`bootcode.bin`) running on the GPU implements
a way to send it messages and receive a response.  If you look through
the [mailbox message writeup][mailbox-messages] you'll see all sorts of
useful facts you can query for --- model number, serial number, ethernet
address, etc.  So it's worth figuring out how to do it.

As you no doubt expect, the mailbox interface is not super-intuitive,
and the main writeup uses passive-declarative voice style that makes it
hard to figure out what to do.  (One key fact: the memory used to send
the request is re-used for replies.)

So we'll do a few examples so you can get a handle on it.

Checkoff:
  1. Get your pi's revision, physical memory size and temperature.  -
  2. Increase your memory size to 496MB.  Make sure the mailbox call 
     returns this value.
  3. Overclock your pi, and write tests that show it goes faster.
     You should at least have the ARM at 1GHz and the other target values
     (check back for these --- we are rerunning experiments.)  Ideally:
     measure how much faster you can get it before things break down.

There are tons of possible extensions.  
  - Maximum speed?
  - Minimum speed?
  - One fun hack: You could see how much it heats up and down-throttle til
    cools off.  You probably want to write some kind of fancy calculation
    that you can check and see when it breaks down.
  - We only scratched the surface of the mailbox interface: figure out
    something interesting and do it!

------------------------------------------------------------------------------
### 0. Background: mailboxes `code/mbox.[ch]`

If you look through the (unfortunately incomplete) [mailbox
writeup][valvers] you'll see all sorts of useful facts you can query for
--- model number, serial number, ethernet address, etc.  So it's worth
figuring out how to do it.

So that's what we will do.  
 - NOTE: As always: if you want to write the code completely from scratch,
   I think doing so is a valuable exercise.  However, in the interests
   of time I put some starter code in `code/`.  Extend it to query
   for physical memory size along with a couple of other things that
   seem useful.

Rules:
  1. Buffer must be 16-byte aligned (because the low bits are ignored).
  2. The response will overwrite the request, so the size of the buffer
     needs to at least be the maximum of these.
  3. The document states that additional tags could be returned --- it may
     be worth experimenting with sending a larger buffer and checking
     the result.

##### Example: getting the board serial number

If you read through the mailbox document, there's a bunch of cool little
factoids you can get (and sometimes set).  One is the unique board serial
number. The main useful property of the serial number is uniqueness and
persistence --- no board in this room should have the same serial number
and a given board's serial number does not change from run to run.  

Later this quarter we'll see how unique, persistent host identifiers
are useful in a networked system when you want to have a guaranteed
unique ID for ordering, statically binding programs to boards, etc.
For the moment, let's just try to figure out what ours are.

Searching through the mailbox document we see the "get serial number"
message is specified as:

<p align="center">
  <img src="images/get-serial.png" width="250" />
</p>

Where:
  - the "tag" of `0x00010004` is a unique 32-bit constant used 
    to specify the "get serial" request.  This is a message opcode
    to the GPU (the "video control" or VC) knows the request.
  - The request message size is 0 bytes: i.e., we don't send 
    anything besides the tag.
  - The  response message size is 8 bytes holding the serial number.
    (i.e., two 32-bit words). 

Ok, how do we send it?  The start of the document has a somewhat
confusingly described message format.

The first part is fairly clear:
<p align="center">
  <img src="images/msg-format.png" width="550" />
</p>

For our "get serial" message we'll need a 8 word message (calculated
below) where each word in the message `msg` will be as follows:

  - `msg[0] = 4*8`: The first 32-bit word in the message contains the 
     size of tte message in bytes.   For us: as you'll see the 
     message has 8 words, each word is 4 bytes, so the
     total size is `4*8`.
  - `msg[1] = 0`: the second 32-bit word in the message contains
    the request code, which is always 0.  After the receiver writes
    the reply into the `msg` bytes, `msg[1]` will either change to
    `0x80000000` (success) or `0x80000001` (error).

  - We then fill in the tag message.
     - `msg[2]= 0x00010004` (our tag: specified by the doc).
     - `msg[3] = 8`: the response size in bytes: the serial
       is two 32-bit words so this is 8 bytes.
     - `msg[4] = 0`: the document states we write 0 (always) for a request.
       After a sucessesful send and reply, `msg[4]` should hold the
       constant `0x80000008` which is the result of setting bit 31 to 
       1 (as stated in the doc) and or'ing this with the 
       the response size (again: 8, from the doc). (i.e., `(1<<31)
       | 8)`).
     - `msg[5] = 0`: since the reply message is written into our send
       buffer `msg` we need to pad our message so it contains enough space
       for the reply
     - `msg[6] = 0`: again, padding for the second word of the reply.
  - `msg[7] = 0`: The final word is `0`.  

To clean things up some:

    // declare a volatile buffer, 16 byte aligned.
    volatile uint32_t __attribute__((aligned(16))) msg[8];

    msg[0] = 8*4;         // total size in bytes.
    msg[1] = 0;           // sender: always 0.
    msg[2] = 0x00010004;  // serial tag
    msg[3] = 8;           // total bytes avail for reply
    msg[4] = 0;           // request code [0].
    msg[5] = 0;           // space for 1st word of reply 
    msg[6] = 0;           // space for 2nd word of reply 
    msg[7] = 0;           // end tag [0]

Ok, we have a buffer, how do we send it?

Well, the broadcom 2835 has no discussion of the mailbox  other than
the ominmous statement: "The Mailbox and Doorbell registers are not for
general usage." (page 109).  So you can either piece things together
from linux source, or different random web pages.  We'll use the 
valver's discussion which is pretty nice:
  - [mailbox discussion][valvers]

Just search for "mailbox".

There are three addresses we care about:

```c
    #define MBOX_STATUS 0x2000B898
    #define MBOX_READ   0x2000B880
    #define MBOX_WRITE  0x2000B8A0
```

For status, we have two values:

```c
    #define MAILBOX_FULL   (1<<31)
    #define MAILBOX_EMPTY  (1<<30)
```

The model here is that you send and receive messages to the GPU.  As you
saw in 140e, whenever sending and receiving data between different
hardware devices:

  - For send: we won't have infinite buffering and so need to check
    if there is space.  In our case we wait until:

        while(GET32(MBOX_STATUS) & MAILBOX_FULL)
            ;

    We can then send by writing the address of the buffer to `MBOX_WRITE`
    bitwise-or'd with the channel we send on (the document states this
    should be 8).

    This is why the buffer must be aligned: so the lower 4 bits are zero
    and so can be reused for the channel id.

  - Similarly, requests are not instantanous, and so we need to
    wait until they return before reading any response.  In our case we
    wait until `MBOX_STATUS&MAILBOX_EMPTY` is 0:

        while(GET32(MBOX_STATUS)&MAILBOX_EMPTY)
            ;

    We then read the response using `GET32(MBOX_READ)`.  The value should
    have the mailbox channel (8) in the low bits.

  - Finally: when writing to different devices we need to use device
    barriers to make sure reads and writes complete in order.
    The simplest approach: do a device barrier at the start and end of
    each mailbox operation.


The `code` directory has a complete version:
```c
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

    // send and receive message
    mbox_send(MBOX_CH, msg);

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
```

-------------------------------------------------------------------
### 1. Get your model, revision, RAM size, temperature.

We'll start with several messages;
  1. `rpi_get_model`: get your board's model.
  2. `rpi_get_revision`: get your board's revision.  You can check that
     this value makes sense from the [board revision page][pi-revisions].
  3. `rpi_get_memsize`: get your board's memory size --- the board has
     512MB but the GPU claims a bunch by default. (I *believe* should
     be 128MB for us.)
  4. `rpi_temp_get`: get your board's temperature.  (FWIW: I had around 90F.)

We gave out pi's mostly from the same digikey batch, so these should
*probably* be the same as your partner's, but they might not be.

-------------------------------------------------------------------
### 2. Update firmware so can increase memory size (and overclock)

By default our firmware only gives us 128MB of RAM out of the total 512MB
the the pi.  We'll change the firmware so we can get 496MB of physical
memory of the 512MB and leave the GPU with only 16MB.  The more memory,
the more fun during the virtual memory labs!

Initially, we got this cut-down firmware because it was reputed to let
us grab the most RAM.  However, interestingly, it also lets us crank
overclocking up much more than the other firmware versions I tested
(5 or 6 versions).
  - NOTE: (There might be a better one out there though --- let us
    know if you find one.

*What to do*: You can either:
  1. do the steps in the [original 140e lab][140e-firmware]
     to get the firmware so you know how to find such things, or: 
  2. just copy the firmware and `config.txt` from `firmware-increase-mem/`
     to your microSD card.  Note the GPU memory configuration in the
     `config.txt` --- you can also increase the maximum allowed bcm2835
     frequency (`core_freq`) and CPU frequency (`arm_freq`) for the
     next part.
  3. Verify that you have 496MB.  The next lab and the others depend
     on this!

-------------------------------------------------------------------
### 3. Overclock your pi!

***NOTE: DO A PULL IF YOU SEE THIS --- we are adding specific targets***
***NOTE: DO A PULL IF YOU SEE THIS --- we are adding specific targets***
***NOTE: DO A PULL IF YOU SEE THIS --- we are adding specific targets***

Fun: how fast (or slow?) can you make your pi?

Implement:
 - `rpi_clock_curhz_get(clk)`: gets the current frequency for 
   `clk` (e.g., sdram, core, cpu, etc).
 - `rpi_clock_maxhz_get(clk)`: get the maximum value for `clk`.
 - `rpi_clock_realhz_get(clk)`: get the measured value for `clk`.
 - `rpi_clock_hz_set(clk, hz)`: set `clk` to `hz`.

What to do: 
  1. For each interesting clock: Get the current clock speed.  
  2. Get the maximum clock speed (should match `config.txt`).
  3. Set the clock to the maximum (or less).
  4. Check that the changes actually happened: there are measured clock
     mailbox calls.  
  5. As an extension: See how low you can set the clocks.  I just 
     realized we haven't tried this, so am curious how it goes.

You should also write code to measure how fast the pi goes after
overclocking.
  1. Cycles per second: can so compute how many cycles per second by 
     using `cycle_cnt_read()` and `timer_get_usec()`.
  2. Compute how many GPIO or memory operations can be done.
  3. We give some measurement example code in [0-measure](0-measure/README.md).
     I'm curious how different pieces speed up.
  4. When over-clocking goes too far it can start corrupting bits rather
     than crash outright.  To check correctness you should 
     use an adaptation of the sha-hashing computation from 
     lab 4 to (hopefully) detect when the code breaks down exactly.

**Tips**:
  - I had to change the `arm_freq=700` value in my `config.txt`
    before I could make the pi run faster (otherwise it ignored the
    changes).
  - There are other clocks besides the ARM CPU (`arm_freq`) that you
    can set.  The primary ones: `gpu_freq`, `core_freq`, `sdram_freq`.
    There's likely a bunch of other settings (or other interesting
    firmware) that can push you even further.
  - Generally: if you increase speed you may have to increase voltage.
    E.g., over-voltage the sdram (e.g., `over_voltage_sdram=4`) or the
    entire board (`over_voltage=6`).
  - These are all highly arbitrary; I don't know a one-stop-shop of the
    "best" settings and/or their values.  Let us know if you find
    something interesting!

**Common error: UART garbage**:
  1. Once you change the BCM2835 frequency you'll have to either change
     your uart speed by changing the divisor (easy hack: make a new
     `uart_init_div` that takes a divisor and re-initializes the uart)
     bitbang the uart, or write a pl011 driver (useful for LX).  The
     `code/` directory has an example for how to make output use the 
     staff PL011 driver.  One downside: because it has a slower clock,
     it can't support as high an overclock as the miniUART.
  2. You'll see this because you'll overclock and start getting garbage
     output.

I'm curious how fast you can make these go.

[valvers]: https://www.valvers.com/open-software/raspberry-pi/bare-metal-programming-in-c-part-5/#mailboxes
[mailbox-messages]: https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
[140e-firmware]: https://github.com/dddrrreee/cs140e-22win/blob/main/labs/10-low-level/increase-mem/README.md
[pi-revisions]: https://www.raspberrypi-spy.co.uk/2012/09/checking-your-raspberry-pi-board-version/
