#ifndef __PIX_INTERNEL_H__
#define __PIX_INTERNEL_H__

#define MB(x) ((x)*1024*1024)

// start putting config options in here.
typedef struct config {
#if 0
    // if != 0: randomly flip a coin and 
    // Pr(1/random_switch) switch to another 
    // process in ss handler.
    uint32_t random_switch;

    // if 1: turn on cache [this will take work]
    unsigned do_caches_p:1;

    // if 1: turn vm off and on in the ss handler.
    unsigned do_vm_off_on:1;

    // if 1, randomly add to head or tail of runq.
    unsigned do_random_enq:1;

    unsigned emit_exit_p:1; // emit stats on exit.

    unsigned no_vm_p:1; // no VM
#endif
    unsigned verbose_p:1,
             icache_on_p:1,
             compute_hash_p:1,
             vm_off_p:1,
             run_one_p:1,
             hash_must_exist_p:1,
             disable_asid_p:1;

    unsigned debug_level;     // can be set w/ a runtime config.
    unsigned ramMB;           // default is 128MB
} config_t;

extern config_t config;

#define pix_debug(msg...) do {          \
    if(config.verbose_p)                \
        debug(msg);                     \
} while(0)

enum {
    PIX_OFF = 0,
    PIX_HIGH = 1,

    PIX_MEDIUM,
    PIX_LOW,
    PIX_ALL,

    // only print high priority statements
    PIX_DEBUG_LEVEL = PIX_HIGH 
};

#define pix_debug_lvl(lvl, args...) do {        \
    if((lvl) <= config.debug_level)                \
        debug(args);                            \
} while(0)

#endif
