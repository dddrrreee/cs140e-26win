// really simple "OS" that:
//  1. loads external programs (see the weird <_cstart>)
//  2. runs each program in a single 1MB page address space 
//     with pinned
//  3. runs them with single stepping and computes register hashes
//  4. on each exit checks that the exit hash matches what is 
//     expected.
//  5. handles fork, waitpid (sort of) exit, and various print
//     routines --- you can add others!
//  
// the hashes are pre-computed and stored in a table.  
//
// downside: the code should be really simplified and redone (sorry :/)
// however the algorithm for doing so is simple: just use epsilon
// induction:
//   1. run all the programs and make sure the hashes pass.
//   2. make the smallest change that you can.
//   3. rerun and make sure hashes pass.
//   4. repeat.
//
// suggested order:
//  1. treat the OS as a big test case.
//  2. start swapping in your code for ours --- pinned, 
//     single stepping, switchto.   
//  3. then start making it more complete, faster, etc.
#include "rpi.h"
#include "switchto.h"
#include "pix.h"

#include "fast-hash32.h"
#include "pix-internal.h"
#include "breakpoint.h"
#include "small-prog.h"

#include "syscall-num.h"

#include "pix-internal.h"
config_t config = {
    .verbose_p = 0,
    .icache_on_p = 0,
    .disable_asid_p = 0,
    .ramMB = 150,
    .run_one_p = 0,
    .compute_hash_p = 1,
    .vm_off_p = 0
};

const char * hash_name_lookup(uint32_t prog_hash);


//******************************************************
// simple 1MB allocation/deallocation.  reference counted.
//
// this code can be done in 10-20 lines, sorry --- good
// candidate to setup.

enum { MAX_SECS = 512 };  // can't ever be bigger than this.
static uint32_t sections[MAX_SECS];
// actual number of 1mb sections [will be smaller than 512mb] 
static uint32_t nsec;

// need to use the mbox to read the 
// the amount of actual memory avail.
static void sec_alloc_init(unsigned n) {
    assert(n>0 && n < MAX_SECS);
    nsec = n;
}

// within [0..nsec)
static int sec_is_legal(uint32_t s) {
    return s < nsec;
}

// we are using physical address.  not sure.
static int sec_is_alloced(uint32_t pa) {
    // allocated by the machine.
    // unclear we should do this.
    if(pa >=  0x20000000)
        return 1;

    uint32_t s = pa >> 20;
    if(!s)
        assert(!pa);
    assert(sec_is_legal(s));
    // refcnt not 0 = allocated.
    return sections[s];
}

// allocate 1mb section <s> --- currently
// panic if not free.
static int sec_alloc_exact(uint32_t s) {
    assert(sec_is_legal(s));
    if(sections[s])
        return 0;
    sections[s] = 1;
    return 1;
}

// allocate a free 1mb section.
static long sec_alloc(void) {
    for(uint32_t i = 0; i < nsec; i++) {
        if(!sections[i]) {
            sections[i] = 1;
            return i;
        }
    }
    // change this to an error.
    panic("can't allocate section\n");
    return -1;
}

// returns refcnt
static long sec_free(uint32_t s) {
    assert(sec_is_legal(s));
    if(!sections[s])
        panic("section %d is not allocated!\n");
    return --sections[s];
}


//*************************************************************
// pinned vm lab.

#include "pinned-vm.h"
#include "pointer-T.h"

#define dom_mask(x)  (DOM_client << ((x)*2))
enum {
    dom_kern = 1,
    dom_user = 2,
    dom_mask = dom_mask(dom_kern) | dom_mask(dom_user)
};

enum {  kern_priv = perm_rw_user };

static unsigned n_pin;
static void *null_pt;

static inline uint32_t sec_to_addr(uint32_t sec)
    { return (sec << 20); }
static inline void* sec_to_ptr(uint32_t sec)
    { return (void*)sec_to_addr(sec); }
static inline uint32_t addr_to_sec(uint32_t addr)
    { return addr >> 20; }
static inline uint32_t ptr_to_sec(void *ptr)
    { return addr_to_sec((uint32_t)ptr); }
static inline uint32_t ptr_to_off(void *ptr)
    { return (uint32_t)ptr & ((1<<20)-1);}

// what is asid?
static void 
pin_map(unsigned idx, uint32_t va, uint32_t pa, pin_t attr) {
    assert(sec_is_alloced(pa));

    // output("MAP: idx=%d: %x->%x\n", idx,va,pa);
    assert(idx<8);
    staff_pin_mmu_sec(idx, va, pa, attr);
}

// we pin right away.
void pin_ident(unsigned idx, unsigned addr, pin_t attr) {
    uint32_t secn = addr >> 20;
    assert(sec_is_legal(secn));
    if(!sec_alloc_exact(secn))
        panic("can't allocate sec=%d\n", secn);

    pin_map(idx, addr, addr, attr);
}

// we use 16MB sections for the kernel so can map as much
// as possible with as few entries as possible.
static inline pin_t pin_set_16mb(pin_t p) {
    p.pagesize = PAGE_16MB;
    return p;
}

// basic idea: memory map the kernel.   this should remind
// you of pinned vm --- its the same setup.
static void pin_vm_init(void) {
    assert(!config.vm_off_p);
    assert(!config.vm_off_p);
    // pinned page table.
    null_pt = kmalloc_aligned(4096 * 4, 1 << 14);
    assert(is_aligned(null_pt, (1<<14)));

    sec_alloc_init(config.ramMB);

    // suggested change: move kernel around so null traps.
    pin_t kern_attr = pin_mk_global(dom_kern, kern_priv, MEM_uncached);
    pin_ident(n_pin++, 0, kern_attr);
    pin_ident(n_pin++, MB(1), kern_attr);

    // suggested change: redo the stacks so we don't
    // need so much stuff and can collapse so there
    // are more pins available for user-processes.
    pin_ident(n_pin++, STACK_ADDR-MB(1), kern_attr);
    pin_ident(n_pin++, INT_STACK_ADDR-MB(1), kern_attr);

    // we map all the device memory using a single 16MB mapping.
    pin_t dev_attr = 
        pin_set_16mb(pin_mk_global(dom_kern, kern_priv, MEM_device));
    pin_map(n_pin++, 0x20000000, 0x20000000, dev_attr);
}

static void pin_vm_on(void) {
    assert(!config.vm_off_p);
    assert(!staff_mmu_is_enabled());

    // one time init.
    staff_pin_mmu_init(dom_mask);
    // kernel is identity mapped: must already be setup.
    staff_pin_mmu_enable();
    assert(staff_mmu_is_enabled());

}

//*************************************************************
// process code.

#define MAX_PINS 3
#define MAX_KIDS 8
#define MAX_NAME 64

#include "mmu.h"

// generate a process queue implementation.
//
// we have to do the decl first b/c the 
// <proc> structure has an internal queue of 
// type <pq_t>.
#include "queue-ext-T.h"
gen_queue_decl(pq, pq_t, struct proc, next)

// process
typedef struct proc {
    regs_t      regs;
    char  name[MAX_NAME];
    uint32_t    pid;        // pid -- monotonically increasing.
    uint32_t    asid;       // current ASID.  

    // process status: 
    // - if can run is <PROC_RUNNABLE>, 
    // - if waiting for something = <PROC_BLOCKED>, 
    // - if exited = <PROC_EXITED>
    enum { 
        PROC_RUNNABLE = 0, 
        PROC_EXITED = 2, 
        PROC_BLOCKED = 3 
    } status;

    // all the processes waiting on us to exit: should put
    // them on the runq when you exit.
    pq_t exitq;

    // we do this very hacked: add a block queue.
    long        exitcode;
    
    // if we have kernel threads, may be worth 
    // sharing this?  
    struct pins {
        uint32_t pa;
        uint32_t va;
        pin_t attr;
    } pins[MAX_PINS];
    unsigned npins;

    struct proc *next;      // used by runq
    struct proc *parent;    // who forked us.
    uint32_t nkids;         // number of kids we forked.

    uint32_t hash_regs;     // single step hash of all registers
    uint32_t hash_pc;       // hash of all the pc values (used for lookup)
    uint32_t inst_cnt;      // how many instructions process ran.
    uint32_t code_hash;     // hash of the code segment 

    // idk if we should have a list or array of the kids?
    // array makes the large fork examples work better.  tho
    // they might be small numbers.
    struct proc *kids[MAX_KIDS];
} proc_t;

static void equiv_onexit(proc_t *p);

gen_queue_fn(pq, pq_t, struct proc, next)
static pq_t runq;

static proc_t *volatile curproc;
static int npid = 1024;

void safe_strcpy(char *dst, const char *src, unsigned n) {
    // copy at most <n>-1 bytes.
    for(int i = 0; i < n-1; i++) {
        // if we hit the terminator, return.
        if(!(dst[i] = src[i]))
            return;
    }
    dst[n-1] = 0;
}

// allocate an asid: currently is fatal if we run out.
// suggested change: be smarter!   
//  1. kill all the ASIDs and redo.
//  2. flush just the reallocated ASID.
//
// note: it doesn't work to just rely on the fact that
// we always write to the same pin offset --- at least
// for my tests, i'd get bad interactions.  i think b/c
// they were cached in the micro-tlbs.
static inline int asid_get(void) {
    if(config.disable_asid_p)
        return 1;

    static int asid = 1;
    asid++;
    // XXX: need to make it so we can restrict ASIDs
    // more in order to test coherency code.
    demand(asid < 64, need to clear the TLB);
    return asid;
}

// make a new process structure.
static inline proc_t proc_mk(const char *name) {
    proc_t p = {
        .asid = asid_get(),
        .pid = ++npid, 
        .status = PROC_RUNNABLE
    };
    safe_strcpy(p.name, name, sizeof p.name);
    return p;
}

static inline proc_t *proc_new(const char *name) {
    proc_t *p = kmalloc(sizeof *p);
    *p = proc_mk(name);
    return p;
}

// add a pin to process <p>
static inline void 
proc_pin_add(proc_t *p, uint32_t va, uint32_t pa, pin_t attr) {
    assert(p->npins < MAX_PINS);

    p->pins[p->npins++] = (struct pins) { 
        .va = va, 
        .pa = pa, 
        .attr = attr 
    };
}

// assume kernel pins stay sticky.
static inline void proc_map_pins(proc_t *p) {
    let n = p->npins;
    assert((n_pin + n) < 8);

    for(int i = 0; i < n; i++) {
        let pin = &p->pins[i];
        pin_map(n_pin+i, pin->va, pin->pa, pin->attr);
    }
    assert(p->asid);
    // suggested change: use your real pinned vm routines.
    staff_set_procid_ttbr0(p->pid, p->asid, null_pt);
}

static void schedule(void) {
    // should print out total runtime etc.
    if(pq_empty(&runq)) {
        output("no more threads: reboot!\n");
        clean_reboot();
    }

    let p = pq_pop(&runq);
    uint32_t pc = p->regs.regs[15];
#if 0
    output("switching to <%s>:pid=%d,pc=%x\n", 
            p->name, 
            p->pid,
            pc);
#endif
    proc_map_pins(p);
    curproc = p;
    if(config.compute_hash_p)
        brkpt_mismatch_set(pc);
    switchto(&p->regs);
}

// copy 1MB section <src> to <dst>.  we use an exceptionally
// dumb algorithm:
//   1. turn off VM.
//   2. copy it.
//   3. turn back on.
// likely the most expensive operation we do.
//
// if you use a page table for the kernel, you can 
// alias all physical memory into a contiguous range
// (make sure is privileged) and copy using that.
//
// if you stick with pinned and you have < 8, then
// you *could* add pin and remove it, but you need
// to call your coherence (this seems to map to PTE 
// modification).  if you don't it appears you can
// get unexpected results (but hopefully your
// hashes will vail fail.)
//
// at some level the easiest: use DMA, which use
// bus addresses, so will work even with VM on.
// there's a rough 340lx lab that gives DMA examples.
static void copysec(uint32_t dst, uint32_t src) {
    // suggested change: very expensive!  remove.
    staff_mmu_disable();
    memcpy((void*)dst, (void*)src, MB(1));
    staff_mmu_enable();
}

// duplicate <src> into a 1MB section and return it.
static inline uint32_t sec_dup(uint32_t src) {
    uint32_t dst = sec_to_addr(sec_alloc());
    copysec(dst, src);
    return dst;
}

// need to figure out the right way to handle waitpid when 
// the process has exited
int sys_waitpid(long pid, uint32_t options) {
    // right now can't wait on parent.
    if(pid == 0)
        todo("add code so can wait on parent\n");
    if(pid < 0)
        todo("add code so can wait on ancestors\n");

    let p = curproc;
    if(pid > p->nkids)
        todo("add error handling for illegal pid=%d (max=%d)\n",
                    pid, p->nkids);

    proc_t *k = p->kids[pid-1];
    assert(k);
    if(k->status == PROC_EXITED) {
        output("WAITPID: proc=%d: kid=%d exited, returning: %d\n", 
            p->pid, k->pid, k->exitcode);
        return k->exitcode;
    }

    if((options & WNOHANG) == WNOHANG) 
        return 0;
    assert(!options);

    // suggested change: have specific configs to turn this
    // nonsense off and on.
    output("WAITPID: proc=%d: blocking on kid=%d\n", 
            p->pid, k->pid);
    p->status = PROC_BLOCKED;
    pq_append(&k->exitq, p);
    output("\tWAITPID: kid: pid=%d [waiters=%d]\n", 
        k->pid, pq_nelem(&k->exitq));
    assert(pq_nelem(&k->exitq));
    schedule();
    not_reached();
}

// fork address space: if you have pointers to
// resources (like pipes) have to increase
// their reference counts.
static int sys_fork(void) {
    assert(!config.vm_off_p);

    proc_t *p = kmalloc(sizeof *p);
    *p = *curproc;

    p->pid  = ++npid;
    p->nkids    = 0;
    p->asid = asid_get();

    let cur = curproc;

    assert(cur->nkids < MAX_KIDS);
    cur->kids[cur->nkids++] = p;

    // setup pins: we copyied, can skip.
    unsigned npins = p->npins;
    assert(npins);

    // can we do without asid?
    // if you don't do an asid when you switch you
    // better nuke alot of stuff.
    for(int i = 0; i < npins; i++) {
        let dst = &p->pins[i];
        // these should never be 0.
        assert(dst->va);
        assert(dst->pa);
        dst->pa = sec_dup(dst->pa);
        dst->attr.asid = p->asid;
    }

    p->regs.regs[0] = 0;
    pq_append(&runq, p);
    return curproc->nkids;
}

static void sys_exit(int exitcode) {
    // wait: this just loses the process.   
    // we have to put it on an exitq or something.
    let p = curproc;
    output("EXIT: <%s>:pid=%d, exitcode=%x [waiters=%d]\n", 
        p->name, p->pid, exitcode, pq_nelem(&p->exitq));
    p->exitcode = exitcode;
    p->status = PROC_EXITED;
    equiv_onexit(p);

    proc_t *w;
    while((w = pq_pop(&p->exitq))) {
        assert(w->status == PROC_BLOCKED);
        output("EXIT:WAITPID: proc=%d marking runnable: exit=%d!\n", 
            w->pid, p->exitcode);
        w->status = PROC_RUNNABLE;
        // write the exit code in the waiting proceses reg0
        // (the register that holds the return)
        w->regs.regs[0] = p->exitcode;
        pq_append(&runq, w);
    }

    // XXX: right now we don't free.

    schedule();
}


// very low-rent exec: given a pointer to the process
// binary, setup the address space.
void exec_internal(small_prog_hdr_t *s) {
    // ensure that code and data are aligned to 1MB.
    assert(s->data_addr % MB(1) == 0);
    assert(s->code_addr % MB(1) == 0);

    small_prog_hdr_print(*s);

    void *code_src = (void*)s + s->code_offset;
    void *data_src = (void*)s + s->data_offset;

    uint32_t code_hash = fast_hash32(code_src, s->code_nbytes);
    const char* prog_name = hash_name_lookup(code_hash);
    output("EXEC: progname=<%s>, nbytes=%d\n",
                prog_name, s->code_nbytes);

    let p = proc_new(prog_name);
    p->code_hash = code_hash;

    // XXX: currently: vm not turned on, so we can copy whatever.
    uint32_t data = 0, code = 0;

    // note: if we aren't going to turn on at all,
    // need to map the exact sections.
    if(config.vm_off_p) {
        data = s->data_addr;
        code = s->code_addr;
    } else {
        data = sec_to_addr(sec_alloc());
        code = sec_to_addr(sec_alloc());

        // suggested change: handle recycling ASIDs.
        pin_t user_attr = 
            pin_mk_user(dom_user, p->asid, perm_rw_user, MEM_uncached);
        proc_pin_add(p, s->data_addr, data, user_attr);
        proc_pin_add(p, s->code_addr, code, user_attr);
    }

    unsigned offset = s->bss_addr - s->data_addr;
    assert(offset < MB(1));

    // this either needs mmu off, or needs to turn it off.
    gcc_mb();
    memset((void*)data+offset, 0, s->bss_nbytes);
    memcpy((void*)data, data_src, s->data_nbytes);
    memcpy((void*)code, code_src, s->code_nbytes);
    gcc_mb();


    let r = &p->regs;
    r->regs[0] = 0;
    r->regs[REGS_SP] = s->data_addr + MB(1)/2;
    r->regs[REGS_PC] = s->code_addr;
    r->regs[REGS_CPSR] = USER_MODE;
    pq_append(&runq, p);
}

//**********************************************************
// sys call infrastructure.

static int sys_putc(int chr) {
    rpi_putchar(chr);
    return 0;
}

static int sys_put_pid(void) {
    assert(curproc);
    printk("%d", curproc->pid);
    return 0;
}

static int sys_put_hex(uint32_t x) {
    printk("%x", x);
    return 0;
}
static int sys_put_int(uint32_t x) {
    printk("%d", x);
    return 0;
}

typedef int (*sysfn_t)();
static sysfn_t syscalls[SYS_MAX] = {
    [SYS_FORK] = (void*)sys_fork,
    [SYS_EXIT] = (void*)sys_exit,
    [SYS_WAITPID] = (void*)sys_waitpid,
    [SYS_PUTC] = (void*)sys_putc,
    [SYS_PUT_PID] = (void*)sys_put_pid,
    [SYS_PUT_HEX] = (void*)sys_put_hex,
    [SYS_PUT_INT] = (void*)sys_put_int,
};


// should not get called.  suggested change:
// kill current process and run another.
void data_abort_full_except(regs_t *r) {
    panic("should not get this\n");
}
// should not get called.  suggested change:
// kill current process and run another.
void undef_abort_full_except(regs_t *r) {
    panic("should not get this\n");
}


// syscall dispatch
//
// currently user errors are fatal, which doesn't 
// make sense for an OS.
//
// not sure how much it matters for speed that we have 
// to shuffle registers around.   can worry about later.
// 
// what are we supposed to do with the registers?
int syscall_full_except(regs_t *r) {
    uint32_t r0 = r->regs[0], 
             r1 = r->regs[1], 
             r2 = r->regs[2], 
             r3 = r->regs[3];

    unsigned sysnum = r0;
    if(sysnum >= SYS_MAX)
        panic("invalid syscall: %d\n", sysnum);

    sysfn_t syscall_fn = syscalls[sysnum];
    if(!syscall_fn)
        panic("invalid syscall: %d\n", sysnum);

#if 0
    // should have the string name.
    pix_debug("about to do syscall: sysnum=%d, syscall=%x\n", 
            sysnum, syscall_fn);
#endif

    // should we just return it?  is going to be faster.  but
    // in general we need the registers <r> so probably
    // not really make sense.
    curproc->regs = *r;
    r->regs[0] = syscall_fn(r1,r2,r3);
    switchto(r);
}

// switch from process <p> and jump to <r>.
// interface could be better.
static void switchfrom(proc_t *p, regs_t *r) {
    p->regs = *r;
    pq_append(&runq, p);
    schedule();
}

// compute equivalant hash.
void prefetch_abort_full_except(regs_t *r) {
    if(!brkpt_fault_p())
        panic("impossible: should get no other faults\n");
    assert(config.compute_hash_p);

    uint32_t pc = r->regs[15];
    proc_t *p = curproc;

    // reset mismatch: run next instruction.
    brkpt_mismatch_set(pc);

    p->hash_regs = fast_hash_inc32(r, sizeof *r, p->hash_regs);
    p->hash_pc   = fast_hash_inc32(&pc, sizeof pc, p->hash_pc);
    p->inst_cnt++;

    // XXX: need a way to cleanly check different quanta.  e.g., 
    // switch to another process every instruction, switch ever
    // random number of instructions, etc.  definitely need to be
    // able to do timer interrupts at the same time.
   
    // just resume.
    if(config.run_one_p)
        switchto(r);
    // otherwise switch to another.
    // suggested change: can also randomize.
    switchfrom(p,r);
}

//*************************************************************
// simple equivalance checking.

// simplistic table to keep track of all hashes.
struct eqhash {
    struct eqhash *next;
    char     name[128];
    uint32_t prog_hash;
    uint32_t pc_hash;
    uint32_t equiv_hash;
};

gen_queue_full(hashl, hashl_t, struct eqhash, next)
static hashl_t hashl;

// lookup by <prog_hash> and optionally <pc_hash>
static struct eqhash *
hash_lookup(const char *name, uint32_t prog_hash, uint32_t pc_hash) {
    for(let e = hashl_first(&hashl); e; e = hashl_next(e)) {
        if(e->prog_hash != prog_hash)
            continue;
        if(!e->pc_hash || e->pc_hash == pc_hash)
            return e;
    }
    return 0;
}

// get the name using <pc_hash>
const char * hash_name_lookup(uint32_t prog_hash) {
    for(let e = hashl_first(&hashl); e; e = hashl_next(e))
        if(e->prog_hash == prog_hash)
            return e->name;
    return "fixme: no name";
}

static inline void
hash_insert(const char *name, uint32_t prog_hash, uint32_t equiv_hash, uint32_t pc_hash) {
    struct eqhash *e = kmalloc(sizeof *e);
    safe_strcpy(e->name, name, sizeof e->name);
    e->prog_hash = prog_hash;
    e->equiv_hash = equiv_hash;
    e->pc_hash = pc_hash;

    hashl_append(&hashl, e);
    assert(hash_lookup(name, prog_hash, pc_hash));
}

// called on exit to check that the process hash matched.
static void equiv_onexit(proc_t *p) {
    if(!config.compute_hash_p)
        return;

    if(config.verbose_p)
        trace("FINAL: pid=%d: code-hash=%x total instructions=%d, reg-hash=%x\n",
                p->pid,
                p->code_hash,
                p->inst_cnt,
                p->hash_regs);

    let e = hash_lookup(p->name, p->code_hash, p->hash_pc);
    if(e) {
        if(e->equiv_hash != p->hash_regs)
            panic("hash mismatch: expected %x, have %x\n",
                e->equiv_hash, p->hash_regs);
        // suggested change: have a config that will print.
        // output("hash matched: %s=%x\n", p->name, p->hash_regs);

    // we have all this extra code for if you add new programs
    // you can add hashes pretty easily.
    } else {
        output("inserting hash for <%s>=%x\n", e->name, p->hash_regs);
        output("HASH: hash:\n");
        output("HASH:     name = %s,\n", p->name);
        output("HASH:     prog-hash = %x,\n", p->code_hash);
        output("HASH:     equiv-hash=%x\n", p->hash_regs);
        output("HASH:     ;\n");
        output("hash_insert(\"%s\", %x, %x);\n",
            p->name, p->code_hash, p->hash_regs);
        if(config.hash_must_exist_p)
            panic("should have existed: %s:%x\n", p->name, p->code_hash);
    }
}


//*************************************************************
// one time startup code: responsible for taking all the 
// binaries concatenated to the binary and doing an <exec>
// on each one.

#include "memmap.h"

static void *heap_start = 0;
void *program_end(void) {
    assert(heap_start);
    return heap_start;
}

void pix_notmain(small_prog_hdr_t *progs[], unsigned nprogs);

// sleazy hack: we have concatenated a bunch of programs to 
// the end of the program, so copy them out of the bss before
// zeroing it.
void _cstart() {
    void *src = (void*)__data_end__;
    void *dst = (void*)__prog_end__;

    // very gross: move the payload out of the way.
    gcc_mb();
    memmove(dst, src, 1024*256);
    gcc_mb();

    void * bss      = __bss_start__;
    void * bss_end  = __bss_end__;
    memset(bss, 0, bss_end-bss);
    gcc_mb();

    uart_init();

    output("dst=%x, bss=%x\n", dst,bss);
    assert((void*)dst >= bss_end);

    // we should attach a ramfs to the end of the kernel,
    // instead we just concatenate the programs and pull
    // them out using the sleazy hack of looking for the
    // magic cookie.
    small_prog_hdr_t *s = dst;
    unsigned nprogs = 0;
    small_prog_hdr_t *progs[128], *h;

    // if you look in <user-prog/small-proc.ld> you can
    // see the layout --- there's a magic cookie. along
    // with a bunch of offsets and sizes.
    while(s->magic == 0xfaf0faf0) {
        h = progs[nprogs++] = small_prog_hdr_ptr(s);
        dst += h->hdr_nbytes + h->code_nbytes + h->data_nbytes;
        s = dst;
        assert(nprogs < 128);
    }
    debug("nprogs=%d\n", nprogs);

    // XXX: currently hard coding that pix fits on one page.
    // probably should fix this.
    heap_start = dst;
    uint32_t rem = MB(1) - (uint32_t)heap_start%MB(1);
    kmalloc_init_set_start(heap_start, rem);
    output("heap starts at %p, bytes left=%d\n", heap_start,rem);

    pix_notmain(progs, nprogs);
}


#include "vector-base.h"
void pix_notmain(small_prog_hdr_t *prog[], unsigned nprog) {
    extern uint32_t full_except_ints[];
    vector_base_set(full_except_ints);

    // manually add the hashes: these are precomputed before
    // we ran, and checked on exit.  
    // <name>  |   <prog_hash>  | <equiv_hash> | <pc_hash>
    // name = program name.
    // prog_hash = hash of the code (text) segment so that we can
    //             associate it with the final register hash.  
    //             program name is too weak --- if you recompile
    //             with optimization or use a different compiler
    //             model we'd have a different register hash.
    // equiv_hash = hash of all the registers (same as lab 11).
    // pc_hash    = hash of reg 15 values on the run --- this is
    //              used to distinguish the behavior of different
    //              paths (e.g., after a fork)
    hash_insert("user-progs/0-printk-hello.bin", 0x20006c9a, 0xabe01cc6, 0);
    hash_insert("user-progs/0-hello.bin", 0x8caca1c2, 0x67d47534, 0);

    hash_insert("user-progs/1-fork.bin", 0x57d62e35, 0x5f7f600e, 0xd09bfbad);
    hash_insert("user-progs//1-fork.bin", 0x57d62e35, 0xb959726e, 0xe474b674);

    // same program: multiple legal hashs.  differentiate based on the pc hash.
    hash_insert("1-fork-waitpid.bin", 0x2020997a, 0xf40f5d98, 0x9545afd4);
    hash_insert("1-fork-waitpid.bin", 0x2020997a, 0x6bf7272e, 0x88bd430d);
    hash_insert("1-fork-waitpid.bin", 0x2020997a, 0x5eeb02cc, 0x4e03e6fb);

    // better have some program.
    assert(nprog);

    // if vm is off, we can only run one process at a time
    if(config.vm_off_p) {
        assert(config.run_one_p == 1);
        assert(nprog == 1);

        exec_internal(prog[0]);

        assert(pq_nelem(&runq) == 1);
    } else {
        assert(nprog == 1 || config.run_one_p == 0);
        pin_vm_init();

        // exec all the programs.
        for(int i = 0; i < nprog; i++)
            exec_internal(prog[i]);

        pin_vm_on();
    }

    // if we are doing equiv hashing do the mismatch.  
    // note: if you turn this off, i don't think the other 
    // calls are guarded.  good suggested change is to fix
    // them all in a clean way.
    if(config.compute_hash_p)
        brkpt_mismatch_start();     // start mismatching.  

    // will pull process off runqueue and run it.
    schedule();
    not_reached();
}
