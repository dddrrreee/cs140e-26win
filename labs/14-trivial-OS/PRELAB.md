## PRELAB: trivial-OS.

For this lab we will build simple user-level processes.  This involve
extending the trivial system call we did in lab 4 into a multi-process
operating system.

As part of that you'll need to understand some Unix system
calls (since we are going to build them).  Look at the
[useful-examples/README.md](useful-examples/README.md) and go through
the 5 programs in the `Makefile` understanding how `fork`, `waitpid`,
`exit` and `pipe` works.

You also re-read the interrupt (lab 4) readings --- they will make more
sense now that you've built code that does the things they talk about.
And you'll need to have a firm grounding since the next labs build stuff
that uses the different shadow registers, CPSR and SPSR.  If you don't,
the bugs will be astonishing in their "that is impossible" effects:

  - Re-read the armv6 chapter from the interrupts lab
    (`4-interrupts/docs/armv6-interrupts.annot.pdf`).  You're going to
    want to have a good grasp.

    Pay close attention to: the execution modes, which registers are
    shadowed, what's in the `cpsr` register, how to change modes using
    the `cps` instruction since it is faster than our current use of
    read-modify-write with `mrs` and `msr`.  Also, pay attention to
    discussion about how to access the user-level shadow registers from
    privileged code.  (Load and store instructions using the `^` symbol,
    also how to change the `spsr`.)

    It probably helps to re-read the [INTERRUPT CHEAT SHEET](../../notes/interrupts/INTERRUPT-CHEAT-SHEET.md).

  - [mode bugs](../../notes/mode-bugs/README.md): these are examples
    of different mistakes to make with modes and banked registers.
    Should make these concepts much clearer since you can just run
    the code.

### Some useful background

We have already seen different ways of packaging up execution:
  - lab 2: A single thread of control in a blink-level program that starts
    at 0x8000 and runs until reboot or (if it messes that up) power-cycle.
  - Lab 4,8: interrupts as a way to handle exogenous events not
    (necessarily) caused by the current instruction --- timer
    interrupts, device interrupts.
  - Lab 4: exceptions as a way to handle events caused by the currently
    executing thread of control: system calls, divide by zero, memory
    faults (upcoming).
  - Lab 5: non-preemptive threads for running independent but cooperating 
    executions ("threads of control") in the same address space.   
  - And, presumably, in previous classes: Turing machines.

We will now start building simple user-level processes to manage
independent executions that you want to isolate from each other (so a
crash in one does not crash another) and do not trust enough to run with
full kernel privileges.

As you've no doubt noticed, all the different ways of packaging
execution have the same basic pieces:
  1. Some code that specifies what to do.
  2. A pointer to the current instruction.
  3. Some state to record what has been done.

In theory-land:
 1. Code = the state transition table.
 2. Code pointer = tape head.
 3. Memory = tape.

In CS140E these three pieces correspond to:
  1. Code: the code segment laid out by our linker script (e.g.,
     `libpi/memmap`).   But also possibly generated at runtime on purpose (if
     you did the CS240LX dynamic code generation extension lab) or by
     accident (if you corrupted your code or jumped to the wrong place).
  2. Pointer to the current instruction: The arm1176 program counter 
     register r15 (`pc`).
  3. Memory: the data segment (statically allocated data), the heap
     (what kmalloc pulls in), the stack, and the 16 general purpose
     registers + the CPSR.

From this POV, clearly all the different ways we've packaged execution
share the same basic underlying mechanics.  Where they differ is:
 1. The *context* they execute in.  Mostly our context has been
    different *privileged* modes --- supervisor level (where we usually
    start), interrupts and exceptions --- where the code runs in
    God-mode and can do anything include permanently brick the machine.
    But also includes unprivileged execution (user level), where code
    can't issue privileged instructions and (next week) can't
    avoid OS-enforced memory protection.  Most of your code in other classes has
    run at this level, and we will build it out the rest of the quarter.
 2. How they are scheduled.  A scheduler at its most basic = what
    determines the next instruction address to set the pc to?  Our most
    common scheduling has been cooperative scheduling: the code runs
    until it yields (or reboots).  So in a sense the instruction stream
    itself schedules the next instruction --- it's just what comes next.

    But the interrupt hardware is also a scheduler in that it determines
    when to stop a currently running thread of control and what to run
    next (for us: the interrupt handler).  

    With user-processes we will add what people usually think of when you
    talk about scheduling --- some code in a file (e.g., `schedule.c`)
    that uses timer interrupt to interrupt programs and picks the
    next one.

Lab 10 will start by doing simple version user-level processes, which
are the foundational OS primitive (around since the 1950s at least in
CTSS and others).

Doing a complex, new thing with many moving parts is a tautology for
awful.  Fortunately, we can play some tricks to break it down into much
simpler pieces.
  1. The arm 1176 chip used on the r/pi A+ (and the ARMv6 architecture
     in general) has the unusual feature that you can run code at
     user-level *without* having virtual memory.  This lets us entirely
     skip one of the most error prone subsystems.
  2. From CS240LX: we will do a powerful and fairly simple hack using
     the r/pi debugging hardware so that we can immediately catch if
     the user-level code deviates by even a single-bit in a single
     register from its expected behavior.  I cannot imagine writing OS
     code without this trick, and no one else knows about it besides us,
     so this should be fun.
  3. (Also from CS240LX): Then do pinned virtual memory that does virtual
     memory without a
     page table (where much of the complexity comes from).

For lab 10 we will:
  1. Run a single process at user-level (i.e., without kernel privileges),
     but without virtual memory.
  2. Have a simple kernel that just services system calls (no interrupts).
  3. Write simple, but true system calls that go between user and kernel
     space.   (We previously handled the system call exception, but there
     are some additional details to deal with the fact that we do not trust
     the user's stack, etc.)
  4. Add multiple processes that use swapping and hashing for corruption
     detection.

Then in lab 11 and a follow-on: we will
  1. Use single-stepping hardware to ensure there is absolutely no
     deviation for each change we make.  (More in the lab).
  2. Add full preemptive scheduling

Then we will add virtual memory.
