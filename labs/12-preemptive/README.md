## Pre-emptive threads, checked with single step equivalence 

<p align="center">
  <img src="images/pi-ss-equiv.jpg" width="700" />
</p>

By the end of this lab you'll have:
 1. Deeply-audited, pre-emptive context switching that can save and
    restore all 17 ARM registers and switch between privileged modes.
 2. A pre-emptive threads built using it.
And, in a plot twist no one has ever said about such code: It won't
actually be that hard.  In fact, when you're done with lab we'll be
surprised if your code has a bug in it.  (Surprised enough that you
should please come tell me about it!)  This happy state will come from
two simple yet powerful tricks using debug hardware:
  1. You'll verify your full context switching restore code works  ---
     easily -- by using debugging mismatch/match faults to catch execution
     at the *exact* point after your context restore completes and jumps
     to the intended resume `pc` (but before executing the instruction
     at `pc`).  As a result, you can forensically yet leisurely check
     that restore correctly set all registers to their intended values
     no matter what mode you are jumping from or to.
  2. You'll use mismatch faults and register hashing to verify that
     running code preemptively gives exactly the same results as running
     it sequentially start-to-finish --- that every register has exactly
     the same value for each instruction that gets executed.  Given the
     pieces you've built so far, building this "single step equivalence"
     checking won't be hard, but yet will make hard things easy because
     it makes it trivial to exhaustively check against a simple reference.

I don't know of any other kernel that uses such tricks, so you will be
well beyond state-of-the-art, laughing at peasants at the base of the
castle walls.

In any case, while you do that --- some readings:
  - Make sure you look at the [PRELAB.md](PRELAB.md).
  - And the example in [0-srs-rfe-example](0-srs-rfe-example/README.md).

Next week you'll be able to combine this threading system with the
virtual memory you build to make user level processes that can implement
the UNIX system calls `fork()`, `exec()`, `exit()`, `waitpid()` etc.
You'll also use your single-step equivalence code to validate that this
combination works by using it compute the equivalence hash of processes
running without virtual memory and pre-emption and verifying you get
the same hash result when running with virtual memory, pre-emption.

For the rest of this quarter we will frequently use the single-step
equivalance hashing algorithm:
  1. Compute a ongoing register-hash of code as it runs in the simplest
     way possible (e.g., no pre-emption, single-threaded, start-to-finish,
     no virtual memory, etc)
  2. Re-compute the register hash while running the code with as much
     fancy complexity as possible (virtual memory, context-switching at
     every instruction, caches on, etc.).
  3. Flag if the hashes differ at the end.
This approach is the only way we know to have 93 people build their own OS
and make it surprising if the code is broken.  With that said, equivalence
checking isn't verification, but it does validate so thoroughly that
it's interesting to the staff if your code is broken but can pass
equivalence checking.  (You should let us know if this ever occurs!)

#### Checkoff

The specific code you'll write:
  - Your own low-level pre-emptive context switching and exception
    trampoline code that will replace ours from the last couple of 
    labs (`full-except-asm.o` and `staff-switchto-asm.o`).
  - The equivalent hashing code.

Both of these will be fairly short.  The payoff at the end is a simple
pre-emptive thread package that works.

Checkoff:
  1. `make check` passes in `1-code` with all tests.
  2. `make check` passes in `2-code` with all tests.

---------------------------------------------------------------
## Background: pre-emption and context switching.

Preemption refers to forcibly interrupting a computation (for example
using a timer-interrupt or debug exception) with, generally, a strong
possibility of switching to something else.

The threads package you wrote in lab 5 was non-preemptive ("cooperative")
in that threads ran until they yielded the processor (e.g., by calling
`rpi_yield()` or `rpi_exit()`).  Cooperative threading has two big
positives:
  1. It is relatively simple to build: just save and restore the 
     callee-saved registers.  
  2. It is relatively simple to use. By default, non-preemptive thread
     is a non-interruptible, "critical section" that is only ever broken
     up by voluntary yields.  A nice point from Atul Adya et al: by
     default everything in a non-prepemptive system is a critical section.
     In contrast, by default in a preemptive system nothing is.

We need preemption for our OS because we can't generally rely on user
processes to run cooperatively:
  1. Even one infinite loop would lock up the system.  We need 
     to bound how long the user process can run before the kernel gets
     back control.  
  2. Devices won't generally work well.   Many devices have small
     queues (e.g., UART FIFOs) or tight timing requirements (for
     correctness or speed).  If the kernel can only check a device when
     the user yields then at best throughput and latency will suck and
     at worse it would lose data and gain bugs.

Thus, we want to two things:
  1. To be able to run user-processes pre-emptively where they can be
     forcibly interrupted (e.g., by a timer-interrupt, device interrupt
     or debug hardware exceptions).
  2. When we interrupt a process we want to *completely* save its state
     --- all 16 general purpose registers and CPSR --- so we can switch
     to another process by *completely* loading its state --- also,
     all 16 general purpose registers and CPSR.

You've already done many labs where you've interrupted running code
(step 1).  


The main new problem is saving/restoring *all* registers across
modes (callee and caller, as well as the CPSR) rather than just saving:
  - Only the caller-saved registers, as we did for interrupts
    where we returned back to exactly the code that got interrupted rather
    than switching to another thread or process after the interrupt
    was done.  (Recall: we saved caller registers before calling the
    C interrupt handler because the compiler would assume it could
    trash any caller register and we don't know which ones were in use.
    We didn't have to save the callee-saved because the C compiler would
    do so before using them.)
  - Only the callee-saved registers, as we did for cooperative threads
    where because the thread code called the yielding routine itself the
    C compiler would know the caller-saved registers were potentially
    trashed so would save and restore them itself. (Useful question:
    Why did we need to save callee since the C code would?)

Since you've already saved and restored registers before in isolation,
conceptually it's not hard to save both.  However, the ARM has two
complications:
  1. As you no-doubt-recall: ARMv6 has different "modes" for user level,
     kernel level and different exceptions.  Each of mode has private
     copies of some registers (for today: sp and lr).  These are called
     "banked" registers or shadow registers.
  2. When an exception occurs we will be at a different
     mode than the code that got interrupted, making it easy-mistake
     awkward to accesss the shadow registers of another mode.
Thus, you'll have to write code to read and write the registers at one
level from another --- for the most part, the unprivileged user stack
pointer (sp) and return register (lr) from a privileged interrupt context.

#### Banked registers

<p align="center">
  <img src="images/banked-registers.png" />
</p>

#### CPSR layout

<p align="center">
  <img src="images/cpsr-layout.png" />
</p>

---------------------------------------------------------------
## 1. Set and get banked registers (`1-banked/`)

Code to write:
 1. The file `1-banked/banked-set-get-asm.S` contains all the 
    code to write. 
 2. Before modifying anything: `make check` should pass.
 3. Swap out the staff `staff-banked-set-get-asm.o` in the 
    Makefile, implement the code in `banked-set-get-asm.S`, 
    and make sure `make check` still passes.

This lab depends on correctly figuring out low-level machine facts: how
to change modes, how to access registers in one mode from another, etc.
So we'll first do these parts in isolated, easy to debug pieces.

As we mentioned above, in order to save and restore the state of a level
process from privileged mode we'll have read and write the two banked
user-mode registers (`sp` and `lr`).  The set of tests have you implement
the code to read and write these registers in three different ways.

There are four test harnesses.  You don't have to modify them though you
should definitely insert assertions or print statements if things act
weirdly!  Look at the comments in the files and the tests and implement
each part at a time.  This is a pretty mechanical fetch quest.

Recommended to do them in the following order:
  1. `0-cps-user-sp-lr.c`: get the USER SP and LR registers
     by using the `cps` instruction to switch to `SYS_MODE` (which has the
     same registers as USER) and back.  The `0-user-sp-lr-asm.S` file has
     a example for `cps_user_sp_get` but you should fill in the others.

```
        @ 0-user-sp-lr-asm.S: get USER mode's sp and return the 
        @ result in r0.
        MK_FN(cps_user_sp_get)
            cps #SYS_MODE
            prefetch_flush(r1);   @ note we have to use a caller reg
        
            @ USER And SYSTEM share the same sp/lr/cpsr
            mov r0, sp
        
            cps #SUPER_MODE
            prefetch_flush(r1);
        
            bx lr

```

  2. `0-mem-user-sp-lr.c`: do the same as (1) but use the `ldm` and
      `stm` instructions with the carat operator "^" to access the
      user registers.  The routine `mem_user_sp_get` has an example:

```
        @ store the user mode sp into the address passed as the 
        @ first parameter (i.e., in r0)
        MK_FN(mem_user_sp_get)
            @ store the user mode <sp> register into memory
            @ pointed to by <r0>
            stm r0, {sp}^
            bx lr
```

     If you get confused about the semantics don't forget to look closely
     at the test!  (True for all the other parts, too.)


  3. `0-any-mode-sp-lr.c` generalize the mode switching method for
     any mode (other than USER) so that you can read and write registers
     at any privileged mode.

     You should use the `msr` instruction with the `_cxsf` modifier to to set
     the `cpsr` mode using a register:

            msr cpsr_c, r0

     Followed by a prefetch_flush that uses a caller saved register you
     don't care about.  You can see an example of where we use `msr` in
     `libpi/staff-start.S`

You are now able to:
  1. Read and write banked registers of other modes (both privileged 
     and user-level).
  2. Switch between privileged modes.  
We'll now setup the code to switch to user mode.

---------------------------------------------------------------
## Background: the `rfe` instruction.

Before doing full context switching (Part 2, next) we first
show how use the `rfe` instruction to simultaneously set
both the `pc` and `cpsr` at once: useful for switching from privileged
to unprivileged mode given a memory array of saved registers.

Unlike your thread context-switch code you can't switch into a user-level
process by simply setting the `pc` register --- you need to set the
`pc` as well as the `cpsr` (to unprivileged `USER` mode) and do so
simultaneously:
  - If you set `cpsr.mode=USER` *before* jumping to user code you'd get a
    privileged fault (since the `pc` would point to instructions in
    kernel memory, which we will be marking as inaccessible in user mode).
  - If you set `cpsr.mode=USER` *after* jumping to user code you'd
    let the user instruction execute with kernel privileges.

The `rfe` instruction lets you atomically set both at once.

To see how `rfe` works:
  1. RTFM the manual.
  2. Then look at the complete example `0-rfe-example/`.  Make sure the
     comments and output make sense or the next few lines of code could 
     easily take hours.  
  3. If you're super assiduous, the fancier `0-srs-rfe-example/` has
     additional detailed, but I don't think they are strictly required
     for Part 2 below.

---------------------------------------------------------------
## 2. Full context switching (`2-save-restore`)

You'll now write the full save and restore code.  It won't be much
code (less than 20 lines).  The art to this part of the lab is doing
it in a way that will be easy to check.


     

To validate your register saving code: we will use a variation of the
hack you have seen before:
  1. Set all 17 registers to known values;
  2. Invoke a system call.
  3. The system call trampoline (you write this) should save all 17 
     registers in a single 17-entry array (on the exception stack)
     and call the system call handler.
  4. The system call handler checks that the 17 registers are correct
     (match the values in (1)).
  5. Fixed point: Fix (3) until (4) passes.

To validate your register restore code: we will do new hack I came
up with earlier in the quarter that makes this part so simple we've
collapsed two labs into one:
  1. Initialize the 17 entry register block `regs` to known values.
  2. Setup a mismatch fault for any address that isn't equal
     to the pc stored in `regs[15]`.
  3. Call `switchto_user_asm(regs)`, which will "restore" all registers
     including `cpsr` and `pc`.  
  4. The instant (3) sets `pc` and `cspr`, you will get a mismatch fault 
     --- importantly before the instruction at `pc` executes (so it
     won't be able to cause any harm).
  5. The mismatch (prefetch abort) trampoline will save all registers 
     (you write this code) and call your mismatch handler.
  6. The mismatch handler check that the registers passed to it
     are equal the values from (1).
  7. Fixed point: fix (3) and (5) until (6) passes.

After you get kernel-user working, you can do kernel-privileged mode
switching using a similar trick, with the exception (sorry) that you'll
have to use a match fault.  At the end you'll have the full user-kernel
context switching.


What's great about this?
  1. Without it, errors in save/restore go to a black hole.

#### 1. Exception saving code: `1-simple-save-test.c`

You'll write the system call trampoline to save all 17-registers
into a contiguous array on the exception stack.  You write 
two routines, both in `interrupt-asm.S`:
  1. The system call trampline (`syscall_reg_save`).  
  2. Steal the `rfe_asm` routine from `0-rfe-example`.

The basic idea:
 1.  If you look in `notmain`, the code uses `rfe_asm` to run
     `start.S:regs_init_ident` at user level.
```
    uint32_t regs[2];
    regs[0] = (uint32_t)regs_init_ident;
    regs[1] = USER_MODE;
    rfe_asm(regs);
```

 2. `start.S:regs_init_ident` sets all registers to besides cpsr and pc
    to their register number and then does a system call instruction
    (`swi`):
```
    MK_FN(regs_init_ident)
        mov r0, #0
        mov r1, #1
        mov r2, #2
        ...
        mov r14, #14
        swi 1
```
 3. TODO: implement the system call trampoline (`syscall_reg_save`)
    save the 16 general registers, and the cpsr in 
    a 17 entry array (on the stack) and pass the pointer to it
    as the first argument to `syscall_handler`.

 4. `syscall_handler` verifies the register array.
    has the right values and exits.  (i.e., it is one-shot).

The great thing about this test is that while it is not verification,
it also doesn't have many moving parts, so it should be easy to debug.
Iterate until you have the right register values. If you make a mistake
don't stay passive!  Add whatever print statements you need, etc.
When you're done `make check` should pass.

#### 2. User-level register restore: `2-simple-restore-test.c`

You'll now implement user-level register restore (`switchto_user_asm`).

A big problem with this code:
  - If your restore code has a bug, the code often usually jumps into
    a big black hole you will have no visibility into.
  - Very hard to debug since anything you do will perturb the state
    (often can't do anything but stare).
  - Our cute hack: abuse mis-matching!
  - Will make checking extremely simple, few moving parts.


Basic idea (see: `2-simple-restore-test.c`):
  1. Initializes 17-entry register block to known values 
     (as in the original test).
  2. Set a breakpoint mismatch on an illegal address (so any jump to
     user-level immediately faults).
  3. Calls `switchto_user_asm` (you write), which takes a structure 
     holding the 17 entry array, loads them all and does a `rfe`.
  4. As soon as (3) jumps to user level, the CPU will throw a 
     mismatch fault *before* doing anything.
  5. Your prefetch abort trampoline will save all the registers (same
     as your system call trampoline, though with a different lr) and
     call `prefetch_abort_handler`.
  6. The mismatch handler checks these registers:  
     - If they are what you expect, both save/restore worked.
     - If not, you have a bug: easy to iterate since not much code.


As before, all code to implement is in `interrupt-asm.S`:
  - `switchto_user_regs_asm`:  load all 17 registers and jump
     using `rfe`.
  - `prefetch_reg_save`: same as your system call trampoline, except
     the `lr` adjustment is different.


#### 3. User-level register restore: `3-multi-restore-test.c`

You shouldn't have to write code for this test --- it just gives assurance
by doing much much more aggressive validation:
  1. It runs the mismatch N times (default: 2000).
  2. Sets all registers to random values.
  3. Checks that your save and restore set them up.

#### 4. Rewrite (2) and (3) to use matching.

For this, you will take both tests above and write matching versions.
This will:
  1. Test your matching code.
  2. Make you read the tests better :).

You should be able to just copy the mismatch tests, modify them to use
matching, and get the same output.
  1. Do it with our `staff-breakpoint.o` code check `make check`
     works.
  2. Then swap in your `breakpoint.c` and check `make check` still works.

Not much code, but you now have very aggressive checking.

#### 5. Switching between privileged modes `switchto_priv_asm`.

In this part you'll write code to save and restore registers when you're
coming from and going to privileged (not user mode).
 1. The challenge here is that the caret operator `^` *only* loads
    or stores to user mode registers, not to any other (privileged)
    mode, so won't help us here.  So, since you don't have caret,
    what do you do?   The good thing is that `0-user-sp-lr-asm.S`
    has most of the pieces already so you can just use those.
 2. In some sense it is easier to save and restore state between
    privileged modes versus user mode since we can switch back and
    forth however we want, whereas once we go to user mode we need a
    system call to get back.

What to write:
 - `switchto_priv_asm` that will switch-to a privilege mode rather
    than user mode (as `switchto_user_asm` does).
 - pull your `mode_get_lr_sp_asm` from part 1 over so that we
   can patch registers after a fault from privileged mode.

The easiest way to write:
  1. It should work should work for any privileged mode, not just
     a specific hard-coded one so use `msr` not `cps` to change modes.
  2. A `prefetch_flush`.
  3. An `ldm` of all 16 general-purpose registers --- this will load
     the pc and jump.

NOTE: 
 - When you use `msr` you'll notice the 8th bit in `cpsr` is set, 
   but if you use `rfe` it is not.     
 - This appears to be a quirk in the GNU assembler.  In order to 
   force `msr` to set all the bits it appears you need to have the 
   following suffixes:
```
            msr   cpsr_cxsf, <reg>
```

     This is easy because you already wrote a routine that will
     get these two registers from an arbitrary privileged mode:


Gross ARMv6 hack for handling traps from privileged code:
 1. Our trampolines currently assume we came from user mode.
 2. We could write the assembly to check the spsr and load
    sp and lr from the right mode depending on where we came from.
    This would likely add some bugs.
 3. So instead we do the following gross hack: in any exception
    handler that would come from privleged mode, we'll check the spsr
    and patch the registers afterwards.

 4. Fortunately, you don't have to write this code since you already
    did in part 1:

        void priv_get_lr_sp_asm(uint32_t mode, uint32_t *sp, uint32_t *lr)

    So just pull it over into `interrupt-asm.S.  The test will call it.


