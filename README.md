## CS140E: embedded operating systems (Engler, Winter, 2026)

<p align="center">
  <img src="labs/lab-memes/pi-intro.jpg" width="600" />
</p>

Tl;dr:
  - It's a lab class, so [jump right to the labs](./labs/README.md).

CS140E is an introductory operating systems course. It roughly covers
the same high-level material as [CS 212][cs212] (formerly CS 140), but
with a focus on embedded systems, interacting directly with hardware,
and verification. Both courses cover concepts such as virtual memory,
filesystems, networking, and scheduling, but take different approaches
to doing so. By the end of 140E, you will have (hopefully) built your
own simple, clean operating system for the widely-used, ARM-based
[Raspberry Pi][raspberrypi].

If you're debating taking the class:
 - Here is [last year's class for reference][2024]
 - And a short guide on [Should I take CS 140E?](guides/why-140e.md)

We have an unusually strong and varied staff this year.  Should be a
fun quarter :)

- Head TA: Joseph Shetaye (rockets, booms, winning bets)
- TAs:
  - Maximilien Cura (Rust + insane hacks)
  - Aditya Sriram (ox64 riscv + bass)
- Section leads: (All: unpaid volunteers(!))
  - Rohan Ram Chanani + Asanshay Gupta (the GPU guys)
  - James Yu-tang Chen + Sai Ketan Konkimalla (the DOOM! guys)
  - Ron Dubinsky  (Paxos+networking)
  - Stuart Sul (ELF, real world, and top-tier AI)

------------------------------------------------------------------------
### Details

140e is a lab-based class with no explicit lectures.  We will do two
five to eight hour labs each week.  You should be able to complete
almost all of the lab in one sitting.  There will be several homeworks,
that act as mini-capstone projects tying together the preceding labs.

By the end of the class you will have built your own simple, clean OS
for the widely-used, ARM-based raspberry pi --- including interrupts,
threads, virtual memory, and a simple file system.  Your OS should serve
as a good base for interesting, real, sensor-based / embedded projects.

We try to work directly with primary-sources (the Broadcom and ARM6
manuals, various datasheets) since learning to understand such prose is
one of the super-powers of good systems hackers.  It will also give you
the tools to go off on your own after the course and fearlessly build
sensor-based devices using only their datasheets.

This course differs from most OS courses in that it uses real hardware
instead of a fake simulator, and almost all of the code will be written
by you.

After this quarter, you'll know/enact many cool things your peers do not.
You will also have a too-rare concrete understanding of how computation
works on real hardware.   This understanding will serve you in many other
contexts.  For what it is worth, everything you build will be stuff we
found personally useful.   There will be zero (intentional) busy-work.


<p align="center">
  <img src="labs/lab-memes/feynman-simple.jpg" width="400" />
</p>

------------------------------------------------------------------------
#### Who should take this class.

The goal of the class is to help students who are very motivated and/or
very good to go far, quickly.  We focus on building small, simple but
real OS subsystems --- threading, virtual memory, etc --- that can be
used to build many other things.

You should take this class if:

   1. You write systems code well OR (you don't yet write code well
      AND have a lot of time to devote to the class);

   2. AND you find these systems topics interesting.

The people that found the class valuable in the past were entirely drawn
from this demographic.  I would say that if you struggle building stuff
and are not already interested in the topic, then it's probably better
to take a different class.  I would particularly advise against taking
this class if you were looking for an easier way to satisfy a cs212 
requirement.

   1. It always helps, but you do not need any background in hardware
      or OS stuff to do well in the class.  In fact, you don't even need
      to have been trained in CS: one of the best students from the past
      was a physics PhD student with fairly minimal background in coding,
      so we've had fantastic luck with non-CS folks :) With that said,
      the less background you have, the more motivation you'll need.

      By the end of the class you'll have learned how to comfortably do
      many things that may well seem like superpowers and could easily
      serve you well for the next few decades.

   2. If you have background in the "embedded" space, it's worth taking
      b/c you'll learn a bunch of useful but not widely-known tricks
      (I'll pay for your supplies and a pitcher of beer if this claim
      turns out to be false!)

   3. It's also super fun.   The code you'll write can be used to
      as a basis for building many interesting systems.

      As one measure: Last year, I taught two extra follow-on classes
      (cs240lx and cs340lx) "for free" because of all the interesting
      things that came up from cs140e.   In addition, it's caused me
      to write more code than I have since grad school.  At my age,
      that's an unusual result :)

#### What this class is not

A quick skim might falsely pattern-match on a couple of things:

   1. cs140e is *NOT* an easier version of cs212.  In fact, it can
      be quite a bit harder, since we work with raw hardware, and
      primarily use raw datasheets and ARMv6 architecture manuals
      rather than pre-digested text books or simulators.

      I'd say about 1/3 of the teaching evals from last year explicitly
      stated "do not take cs140e as an easier cs212".

      On the plus side, this approach is the adult way to do things,
      so you will be better prepared after the class for operating
      autonomously in the real world without safety nets or help.
      Everything you'll build will be stuff we personally found useful.
      There is no (intentional) busywork.

   2. Similarly, while we do use a raspberry pi for this class, and there
      are tons of blog posts/instructables/classes for novices showing
      how to use the r/pi to do simple things, this class is not that.

      You'll be writing virtual memory systems, SD card file systems,
      threads, i2c device drivers on raw hardware rather than
      cut-and-pasting some python on a library to blinky a light.

      We use the r/pi because its fairly cheap, holds up to electrical
      mistakes pretty well, runs a legit processor with legit hardware
      and is small enough to carry around easily.

With that said, I set aside the entire quarter to work on this course,
so am happy to help people that need it --- some of the biggest success
stories from past years were from students that had an initially large
gap between their understanding and the topic but started to really
"get it" after 4-5 weeks of struggle.


------------------------------------------------------------------------
### Lab policies.

Since we are short-staffed - you are strongly encouraged to help other
people!  We will try to keep a note of who does so effectively, and will
guarantee that you will be pushed up a grade if you are on the border.

   - You should be able to complete almost all of the lab in one sitting.
     If not, you *must* complete the lab within a week.  We will not
     accept a lab after this.

     We had a more flexible policy in the past.  Unfortunately, since
     labs typically build on each other, once people fell behind they
     got seriously lost.

   - To repeat: You *must* complete the lab within a week of it being
     issued.

   - PRELABS: Before each lab there will be some preparatory reading,
     which will typically include a short set of questions you need to
     answer and turn in *before* the lab begins.  This is an attempt
     to make the labs more information-dense by handling basic stuff
     outside of it, and focusing mostly on any tricky details in it.

   - There will be three homeworks total, which will consist of integrating
     previous labs and submitting working code.

   - You can leave lab at any time, but please either be there at the 
     start for when we discuss any important features of the night's
     lab or don't ask questions (we don't have the staff for O(n) 
     repeats).

------------------------------------------------------------------------
### What to do now.

First things first:

 1. clone the class repository:

       git clone git@github.com:dddrrreee/cs140e-26win.git

    You may need to install `git`, create a `github` account and register
    a key with github (if you don't want to do the latter, use the
    `http` clone method).  See the our [git instructions](guides/git.md)
    for more discussion.

 2. Look in the [docs](docs) directory to get a feel for what is there.


[cs212]: https://cs212.stanford.edu
[raspberrypi]: https://www.raspberrypi.org
[2024]: https://github.com/dddrrreee/cs140e-24win
[2023]: https://github.com/dddrrreee/cs140e-23win
[2022]: https://github.com/dddrrreee/cs140e-22win
[2021]: https://github.com/dddrrreee/cs140e-21spr
[2020]: https://github.com/dddrrreee/cs140e-20win
[2019]: https://github.com/dddrrreee/cs140e-win19
[2018]: https://cs140e.sergio.bz


<p align="center">
  <img src="labs/lab-memes/beast-mode.jpg" width="600" />
</p>
