---

marp: true
theme: default
paginate: true
html: true



style: |
section ul,
section ol {
  line-height: 1.2;
  margin-top: 0.1em;
  margin-bottom: 0.1em;
}
section li + li {
    margin-top: 0.1em;
}


---

# Lab 15: low-level MMU (140e:win26)

<!-- Large image across the top -->
<img src="../images/samsara.jpg"
    style="width: 100%; max-height: 75vh; object-fit: contain;
    display: block; margin: 0 auto;" />



---
## Today: get a complete VM system

Replace all of our MMU code.
  1. write: `mmu.c` 
  2. write: `your-mmu-asm.S`
  3. Argue (1) & (2) are correct.  (Harder than it looks!)

Result: 
 - complete working VM system, 
 - none of our MMU code.

Next week:
 1. Page tables.
 2. Get rid of the rest of the `staff-*.o`.

---
## MMU operations: < 20 lines, but very subtle 

- Every year I read  the same 12 pages for hours.
- Every year I figure out new stuff.
  - Also lose in compaction...

---
## The effects of MMU bugs are *hard*

- MMU bugs usually corrupt state many millions cycles later
  - no crash
  - application does something weird, sometimes.  
- Lots of caches (TLB, BTB, prefetch). No hardware coherence.
  - we have to seek and destroy each one.
  - complication: while our code runs inside the caches
- Zero safety nets.
- ARMv6 is a family of chips, do not fab
  - leave lots of details up-to-implementor

---
## Some simplificaitons

Most of the problems devove to two things:
  1. Get rid of stale values.
  2. Make sure an operation has completed.


---
## Get rid of stale values 

Caches:
  - d-cache, i-cache 
  - prefetch buffer: grabs instructions and decodes them.
  - TLB, micro-TLBs: cache page table entries, tagged w/ ASID.
  - BTB: branch target buffer.  

Algorithm:
  1. When you change something,
  2. look for each cache it could effect.
  3. Evict.

---
## Make sure an operation has completed.

Most cases:
 1. If you need A;B order: make sure A done using DSB.
 2. Our device barrier for TLB, cache, BTB.

---
## Some puzzles.

---
### B2.7.7: changing privilege level.

need anything?

    CPS #USER_LEVEL

---
### B2.7.6 writing cp15, cp14

Need anyting?

    // write control reg 1
    mcr p15, 0, r0, c1, c0, 0

---
### B2.7.5 invalidating the BTB

Need anything?

    // armv6-coprocessor.h: FLUSH_BTB
    mcr p15, 0, r0, c7, c5, 6

---
### Self-modifying code  (B2-22)

Note: if not self-modify (where code already has run and now
we change its bits) can eliminate stuff.

Code:

        1: STR rx  ; instruction location
        2: clean data cache by MVA ; instruction location
        3: DSB  ; ensures visibility of (2)
        4: invalidate icache by MVA
        5: Invalidate BTB
        6: DSB; ensure (4) completes --- do we need for (5)?
        7: Prefetch flush

Why?


---
#### ASID / PT change (B2-25)

Code:

        1. Change ASID to 0
        2. PrefetchFlush
        3. change TTBR to new page table
        4. PrefetchFlush
        5. Change asid to new value.
        6. anything else ???

Why?

---
#### PTE modification (b2-23)

Code:

        1: STR rx ; pte
        2: clean line ; pte
        3: DSB  ; ensure visibility of data cleaned from dcache
        4: invalidate tlb entry by MVA ; page address
        5: invalidate btb
        6: DSB; to ensure (4) completes
        7: prefetchflush

Why?
