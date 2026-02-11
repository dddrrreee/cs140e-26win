## The labs

<p align="center">
  <img src="lab-memes/coding.jpg" width="400" />
</p>

Below describes where we're going and why.   The labs themselves have a
lot more prose on each topic.  There is a pointer to where we currently
are: the labs after this point can still see significant revisions.
   - [Last year's summary of what we did](https://github.com/dddrrreee/cs140e-25win/blob/main/labs/what-did-we-learn.md) might be a useful breakdown of where this is all going.
   - [Final project ideas](./final-projects/README.md).
   - [CS240LX 25 Labs as extensions](https://github.com/dddrrreee/cs240lx-25spr):
     usually you can do any 240LX lab (there are many years of them!) as
     a "hard" extension. (Note: these go back 2024, 2023, 2022, 2021, 2020.)
   - [lab-memes](./lab-memes) revolving around code, bugs, jobs, pizza.

---------------------------------------------------------------------
### 0: non-pi hacking

Unlike all subsequent labs, our first two don't use hardware.  They should
give a good feel for whether the class works for you without requiring
a hardware investment.

  - [0-intro](0-intro): the intro (non-lab) lecture, plus some
    machine code examples showing how observability interacts with
    compiler optimization.

  - [1-trusting-trust](1-trusting-trust): Ken Thompson is
    our patron saint of operating systems --- brilliant, with a gift
    for simple code that did powerful things.   We will reimplement a
    simplified version of a crazy hack he described in his Turing award
    lecture that let him log into any Unix system in a way hidden even
    from careful code inspection.

---------------------------------------------------------------------
### 1: Going down to metal (part I)

The first few labs will writing the low-level code needed to run the
r/pi and using modern techniques to validate it.  Doing so will remove
magic from what is going on since all of the interesting code on both
the pi and Unix side will be written by you:


  - [2-gpio](2-gpio):  Two parts.  First, we will give out the
    hardware and make sure it works: [0-pi-setup](0-pi-setup/README.md).

    Second start getting used to understanding hardware datasheets by
    writing your own code to control the r/pi `GPIO` pins using the
    Broadcom document GPIO description.  You will use this to implement
    your own blink and a simple network between your r/pi's.

  - [3-cross-check](3-cross-check): you will use read-write logging
    of all loads and stores to device memory to verify that your GPIO
    code is equivalent to everyone else's.  If one person got the code
    right, everyone will have it right.

    A key part of this class is having you write all the low-level,
    fundamental code your OS will need.  The good thing about this
    approach is that there is no magic.  A bad thing is that a single
    mistake can make a miserable quarter.  Thus, we show you modern
    (or new) tricks for checking code correctness.

---------------------------------------------------------------------
### 2. Execution: threads, interrupts, exceptions (part I)

Execution comes in many forms.  It can be a tricky topic both because
of its thickets of low-level, hardware specific details and because of
how hard mistakes are to debug.  You will build and use four different
versions of execution (threads, interrupts, exceptions and processes)
so that you understand in a depth-perceptive way what is essential and
what is incidental.  We will also do multiple new tricks using these
that work well at finding bugs in novel ways.

After four runs at the architecture manual, you will have a new
comfort with using it.

  - [4-interrupts](4-interrupts/): you will walk through a simple,
    self-contained implementation of pi interrupts (for timer-interrupts),
    kicking each line until you understand what, how, why.  You will use
    these to then implement a simple system call and a version of `gprof`
    (Unix statistical profiler) in about 30 lines.  Finally, you'll write
    a user-level system call implementation (both the code two switch
    from privileged to user mode and the code to handle system calls).

    Perhaps the thing I love most about this course is that because we
    write all the code ourselves, we aren't constantly fighting some
    large, lumbering OS that can't get out of its own way.  As a result,
    simple ideas only require simple code.  This lab is a great example:
    a simple idea, 30 lines of code, an interesting result.  If we did
    on Unix could spend weeks or more fighting various corner cases and
    have a result that is much much much slower and, worse, in terms
    of insight.


  - [5-threads](5-threads):  we build a simple, but functional
    threads package.  You will write the code for non-preemptive context
    switching:  Most people don't understand such things so, once again,
    you'll leave lab knowing something many do not.   This will give you
    a second view of execution (and some depth perception of the topic)
    as well as a more fluent handle on assembly code.

---------------------------------------------------------------------
### 3. Going down to metal (Part II)

We break up the execution labs by building the rest of the low
level code you've been using ---the bootloader and the UART
driver.

  - [6-bootloader](6-bootloader): two of the biggest pieces of code
    we've given you have been the Unix-side and pi-side bootloader code
    (`pi-install` and `bootloader.bin` respectively).  So this lab has
    you implement your own and get rid of ours.


  - [7-uart](7-uart): the last key bit of code we've given you is for
    controlling the UART (`uart.o`); so this lab has you write your own
    and remove ours.

    To do so you'll write your first real device driver --- for the UART
    --- using only the Broadcom document.  At this point, all key code
    on the pi is written by you.  You will use the cross checking code
    from lab 2 to verify your implementation matches everyone else's.

---------------------------------------------------------------------
### 4. Execution: interrupts, exceptions, processes (part 2)

  - [8-device-interrupts](8-device-int):
    if you keep doing this kind of work the single most common fancy
    "OS" type thing you'll likely do in the future is to setup GPIO
    pin interrupts so that you can get notified when a hardware device
    has data.  So, in this lab we'll setup GPIO interrupts and tune how
    we do general interrupts.

    Without interrupts, it's difficult to get networking working, since
    our GPIO pins (and our UART options) have limited space and, thus,
    unless our code checks them at exactly the right time, incoming
    messages will vaporize.

  - [9-mailbox](9-mailbox): we'll get more speed by using the GPU 
    mailbox interface to overclock the pi, bcm2835 and memory.

====>

  - [10-interleave-checker](10-interleave-checker):
    we'll use single-step execution to write a concurrency checker that
    is tiny but mighty.  By the end you'll be able to race condition
    bugs difficult to catch with any other tool I know of.

  - [11-debug-hw](11-debug-hw): This lab will show how you can abuse
    debugging hardware in a new way to catch memory corruption (such as
    null pointer bugs) *without virtual memory*.

---------------------------------------------------------------------


<p align="center">
  <img src="lab-memes/bug.jpg" width="400" />
</p>
