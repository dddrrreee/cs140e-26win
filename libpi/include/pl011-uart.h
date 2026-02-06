#ifndef __PL01_UART_H__
#define __PL01_UART_H__

// should fill in the others.
enum {
    BAUD_115200 = 115200,
};


void pl011_disable(void);


// setup for console 115200 output
void pl011_console_init(void);

int pl011_tx_is_empty(void);
void pl011_flush_tx(void);


// initialize for 115200.
void pl011_uart_init(uint32_t baud);

// same as can_get8
int pl011_has_data(void);

int pl011_can_get8(void);
int pl011_get8(void);

int pl011_put8(uint8_t c);
int pl011_can_put8(void);

// just has a different type signature: calls put8
int pl011_putc(int c);

static inline void pl011_override_putc(void) {
    rpi_putchar_set(pl011_putc);
}

void pl011_rx_only_init(uint32_t baud);


void pl011_bt_init(uint32_t baud);


static inline void pl011_flush_rx(void) {
    dev_barrier();

    // 1. Flush RX
    while (!(*(volatile uint32_t*)(0x20201018) & 0x10)) {
        *(volatile uint32_t*)(0x20201000);
    }
    
    // 2. Small delay
    delay_ms(10);
    dev_barrier();
}
    

static inline int 
pl011_write(const uint8_t *data, unsigned len) {
    pl011_flush_rx();
    for(unsigned i = 0; i < len; i++)
        pl011_put8(data[i]);
    return 1;
}

// we could return -1 as an int, but probably too
// easy to make mistakes such as assigning to
// a uint8_t and losing the return.
static inline int pl011_get8_async(uint8_t *b) {
    if(!pl011_can_get8()) {
        *b = 0xff;
        return 0;
    }
    *b = pl011_get8();
    return 1;
}

static inline int
pl011_wait_for_data(uint32_t timeout_msec) {
    uint32_t start = timer_get_usec_raw();

    let timeout_usec = timeout_msec * 1000;
    while(1) {
        if(pl011_has_data())
            return 1;
        if((timer_get_usec_raw() - start) > timeout_usec)
            return 0;
    }
    not_reached();
}

static inline int
pl011_get8_timeout(uint8_t *b, uint32_t timeout_ms) {
    *b = 0;
    if(!pl011_wait_for_data(timeout_ms))
        return 0;
    *b = pl011_get8();
    return 1;
}

static inline int 
pl011_read_timeout(uint8_t *buf, uint32_t nbytes, uint32_t timeout_ms) {
    for(int i = 0; i < nbytes; i++)
        if(!pl011_get8_timeout(&buf[i], timeout_ms))
            return 0;
    return 1;
}


void pl011_loopback_init(uint32_t baud) ;

void pl011_test_init(uint32_t baud);

#endif
