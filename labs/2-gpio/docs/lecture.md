### pi rules

For today:
  - power cycle after each test (until we get reboot()).
 
Hardware rules:
  - UNPLUG: if pi gets hot.
  - UNPLUG: if smell like smoke, plastic burning, odd.
  - DON'T PUT on METAL!   

    /be me.

    I was working at Cafe Barronne few years back, they have green
    metal tables.  Put my pi on the table to code.  No more pi.

    I was working at Coffeebar last year.  They have copper-topped tables.
    Put my pi on the table to code.  No more pi.

    I was working on the outside tables outside last year.  They are silver
    metal tables.  Put my pi on the table to code.  No more pi.


How to fix quick if pi stops working: 

  - quick check: make sure SD card all the way in.  Make sure USB cable 
    all the way in. retry.

  - if doesn't work: differential debugging + binary search.


  - if pi h/w doesn't work, plug in your partners.

  - if partner's pi doesn't work: laptop issue.  try different USB port.
    otherwise maybe software.

  - if partner's works, something is wrong w/ your hardware.
    start swapping pieces: first sd card (often messed up), then pi
    (maybe mis-soldered), then parthiv (these seem pretty robust)

  - of course: these rules apply as we attach more hardware devices.

  - we give software that you can quickly run (see staff-binaries)


How to keep a working pi working:

  - don't mess with it.  don't constantly pull out SD card, USB cable,
    etc.  just leave it alone.  i've had setups that worked for years.

-----------------------------------------------------------------------
### GPIO

The game:
   - find the right address.  find the right magic value.   

   - where the address is.
   - rule for memory
        - they don't issue read, wait, then go forward.
        - multiple reads outstanding.
        - not tagged with the address so bus goes 2x.

        - so have to put dev barrier.

        - sometimes you turn on device B by using device A.  so
          put a write barrier.

   device generally definitional passive voice.  that's just the 
   way it is.  big improvement: examples.

in general:
    - find the weird addresses.
    - find the weird values to set.
    - see if any erratta.

    - dev barrier
    - initialize to known values [typically not RMW]

    - make sure you know when RMW

    - basic idea:
        - set to input/output/etc RMW: 3 bits.
        - GPIO_OUT for pin
        - GPIO_IN for reading
        - 
        - put PUT32/GET32
        - strip off bits
        - see whether you need to RMW or just W
        - why INPUT/OUTPUT different?


---------------------------------------------------------------------
### Unclassified

Few points we didn't discuss:
  - shared state isn't just variables.
  - PUT32 as a way to fix.
  - during testing not much use, won't get triggered.
  - if it's in a different cache line can work.

Example bugs:
    - if leave volatile off: won't set.  however default might already be.
    - RMW

    - ah: have them write a test that shows they do a read modify write.
    - have them write a couple of their own programs.
    - do the timing?  this is interesting.
    - have them write a program you can control well?
    - pwm?  lines: x = on, y = off (or vice versa)


TODO: 
  - type-based alias analysis.
  - how quickly can we get to a simple OS with processes and vm and equiv?

Other extensions:
  - write in assembly
  - do different light intensity
  - pwm as line drawing
  - 240lx lab on dyanmic code generation.
  - find the weirdness with the alignemnt?
  - do the watchdog?
