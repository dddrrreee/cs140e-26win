// one way test of sending 4 bytes using N-byted ack'd packets.
//
// why: we use NRF in fixed size mode (you can do dynamic) so can
// easily be that what you want to send is smaller than the packet
// size.   
//
// how: copy the payload into struct of max packet size, padded with
// zeros and send using that.  have to extract the payload at the 
// other end.
// 
#include "../nrf-test.h"

// useful to mess around with these. 
enum { ntrial = 1000, timeout_usec = 1000, nbytes = 4 };

enum {
    test_server_addr = 0xe3e3e3,
    // test_server_addr = 0xe7e7e7,
    test_client_addr = 0xe7e7e7,
};

// example possible wrapper to recv a 32-bit value.
static int net_get32(nrf_t *nic, uint32_t *out, int verbose_p) {
    int ret = nrf_read_exact_timeout(nic, out, 4, timeout_usec);
    if(ret != 4) {
        if (verbose_p)
            debug("receive failed: ret=%d\n", ret);
        return 0;
    }
    return 1;
}
// example possible wrapper to send a 32-bit value.
static void net_put32(nrf_t *nic, uint32_t txaddr, uint32_t x, int verbose_p) {
    int ret = nrf_send_ack(nic, txaddr, &x, 4);
    if(ret != 4)
        if (verbose_p)
            panic("ret=%d, expected 4\n");
}
/**

Server
- Send 100 messages
- Send out SWITCH message
- Receive and print all message until END


Client
- Receive and print all messages
- When SWITCH message received, switch to server
- Send out 100 messages
- Send out END message

*/

static void
ping_pong_ack(nrf_t *s, int verbose_p) {
    unsigned server_addr = s->rxaddr;
    unsigned npackets = 0, ntimeout = 0;
    uint32_t exp = 0, got = 0;

    while(!net_get32(s, &got, verbose_p))
        ;
    trace("Got packet (before for)\n");

    for(unsigned i = 0; i < ntrial; i++) {
        if(verbose_p && i  && i % 100 == 0)
            trace("sent %d ack'd packets [timeouts=%d]\n", 
                    npackets, ntimeout);

        delay_ms(1000);
        net_put32(s, test_client_addr, ++exp, verbose_p);
        while(!net_get32(s, &got, verbose_p))
            ;
        trace("Got packet (in for)\n");
    }
    trace("trial: total successfully sent %d ack'd packets lost [%d]\n",
        npackets, ntimeout);
}

void notmain(void) {
    kmalloc_init_mb(1);

    // configure server
    trace("send total=%d, %d-byte messages from server=[%x]\n",
                ntrial, nbytes, test_server_addr);

    nrf_t *s = server_mk_ack(test_server_addr, nbytes);
    // nrf_t *c = client_mk_ack(test_client_addr, nbytes);

    nrf_stat_start(s);
    // nrf_stat_start(c);

    // run test.
    ping_pong_ack(s, 0);

    // emit all the stats.
    nrf_stat_print(s, "server: done with one-way test");
}
