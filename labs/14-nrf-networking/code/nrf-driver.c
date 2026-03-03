// skeleton for nrf driver.   
//    - sorry it's so large.  the device is pretty complicated and we're
//      just doing a single lab.
//
// --------------------------------------------------------------------
// key files:
//  1. nrf.h: <nrf_t> the nic structure and <nrf_config_t> some of the
//     configuration.  
//  2. <nrf-hw-support.h>: many accessors for reading and writing
//     NRF registers and for SPI.  you really want to look at this 
//     to save time.
//  3. <nrf-hw-support.c>: SPI read/write routines, routines to dump the 
//     NRF configuration.
//  4. <nrf-default-values.h> default values for NRF hardware choices.
//     you want to look at to see what we can change and for where
//     to get the requested values.
//  5. <nrf-public.c> simple veneer over the top of NRF interface.
//
// --------------------------------------------------------------------
//  big picture: stay in RX by default.  only go TX to send then back.
//  0. at startup go PowerDown->StandbyI->RX
//  1. stay in RX p22]
//  2. to send: RX -> StandbyI ->TX -> Standby-II [can send multiple]
//      then StandbyI->RX mode to get back into RX.
//
// alot is in comments, but not everything.  if you're going to use the
// comment, should double check that it's accurate!
//
// --------------------------------------------------------------------
// you definitely want to to:
//  1. check anything you can to make sure the state matches what you think
//     it should be
//  2. always follow the state machine in the datasheet.
//  3. general strategy: in RX mode, quickly flip to TX if have something
//     to send, flip back to RX.
//
// --------------------------------------------------------------------
// useful extensions:
// 0. send large packets by fragmenting and reassembling.
// 1. a reliable FIFO that handles duplicates, out of order, corruption,
//    packets from previous sessions (e.g., when one reboots).
// 2. network boot.  this is fun, very useful.
// 3. possible: add interrupts for receiving packets.  don't do much
//    in the handler.  can also let the remote node send "REBOOT"
//    commands so that you can control a big network.
//
#include "nrf.h"
#include "nrf-hw-support.h"

// precompute the rx and tx configs so we can 
// just set them without thinking.
//
// enable crc, enable 2 byte
#   define set_bit(x)       (1<<(x))
#   define enable_crc      set_bit(3)
#   define crc_two_byte    set_bit(2)
#   define pwr_up          set_bit(PWR_UP_BIT)
#   define mask_int        set_bit(6)|set_bit(5)|set_bit(4)
enum {
    // pre-computed: can write into NRF_CONFIG to enable TX.
    tx_config = enable_crc | crc_two_byte | pwr_up | mask_int,
    // pre-computed: can write into NRF_CONFIG to enable RX.
    rx_config = tx_config  | set_bit(PRIM_RX)
} ;

// set <ce> low: we use a helper so can
// do some error checking.
static inline void ce_lo(uint8_t ce) {
    if(ce != 6 && ce != 5)
        panic("expected 6 or 5, have: %d\n", ce);
    // recall: we didn't use any dev barriers in <gpio.c>
    dev_barrier();
    todo("implement this\n");
}

// set <ce> high: we use a helper so we can
// do some error checking.
static inline void ce_hi(uint8_t ce) {
    if(ce != 6 && ce != 5)
        panic("expected 6 or 5, have: %d\n", ce);
    // recall: we didn't use any dev barriers in <gpio.c>
    dev_barrier();
    todo("implement this\n");
}

// initialize the NRF: [extension: pass in a channel]
nrf_t *nrf_init(nrf_conf_t c, uint32_t rxaddr, unsigned acked_p) {
    // to write yours, comment this out and start editing below.
    // if you get weird results later (part 2, part 3 etc), 
    // flip back on.
    // return staff_nrf_init(c, rxaddr, acked_p);

    // start of initialization: go through and handle no-ack first,
    // then ack.  i'd do one test at a time.

    volatile uint8_t val;

    nrf_t *n = kmalloc(sizeof *n);
    n->config = c;      // set initial config.
    nrf_stat_start(n);  // reset stats.

    // configure the spi hardware and ce pin.
    // <nrf-hw-support.c>
    n->spi = nrf_spi_init(c.ce_pin, c.spi_chip);

    n->rxaddr = rxaddr;     // set the rxaddr
    cq_init(&n->recvq, 1);  // initialize the circular queue

    // p22: put in PWR_DOWN so can configure.
    nrf_put8_chk(n, NRF_CONFIG, 0);
    assert(!nrf_is_pwrup(n));

    // disable all pipes.
    nrf_put8_chk(n, NRF_EN_RXADDR, 0);

    if(!acked_p) {
        // disable retran [reg1: NRF_EN_AA]
        // enable pipe 1: 
        //   - NRF_EN_RXADDR
        //   - NRF_SETUP_RETR
        // ** 

        // Auto retransmission (p. 57)
        val = nrf_get8(n, NRF_EN_AA);
        nrf_put8_chk(n, NRF_EN_AA, val & !0b10);

        // Enable ONLY pipe 1 and nothing else (p. 57)
        nrf_put8_chk(n, NRF_EN_RXADDR, 0b10);

        // Disable retransmission (p. 57-58)
        volatile uint32_t steup_retr = nrf_get8(n, NRF_SETUP_RETR);
        nrf_put8_chk(n, NRF_SETUP_RETR, steup_retr & ~0b1111);

        // when done, these should be true.
        // <nrf-hw-support.h>
        assert(nrf_pipe_is_enabled(n, 1));
        assert(!nrf_pipe_is_acked(n, 1));
        assert(!nrf_pipe_is_enabled(n, 0));
    
    } else {
        // p 57 and p75 (protocol)
        // reg1: NRF_EN_AA: both retran pipe(0) and pipe(1) have 
        //    to be ENAA_P* = 1
        // reg=2: NRF_EN_RXADDR: enable pipes --- always enable pipe 
        //    0 for retran.
        val = nrf_get8(n, NRF_EN_AA);
        nrf_put8_chk(n, NRF_EN_AA, val & !0b11);

        val = 0b10;
        nrf_put8_chk(n, NRF_EN_RXADDR, 0b00111111);

        // reg = 4: NRF_SETUP_RETR: set retrans attempt and delay
        // compute NRF_SETUP_RETR using:
        //  - nrf_default_retran_attempts;
        //  - nrf_default_retran_delay [note you have to 
        //    convert to the right encoding]
        // ** p.34 "ARD=500µs is long enough for any ACK payload length in 1 or 2Mbps mode."
        val = nrf_default_retran_attempts | nrf_default_retran_delay / 500;
        nrf_put8_chk(n, NRF_EN_RXADDR, val);


        // todo("fill this in!\n");

        // double check that both are enabled + acked.
        // helpers are in: <nrf-hw-support.h>
        assert(nrf_pipe_is_enabled(n, 0));
        assert(nrf_pipe_is_enabled(n, 1));
        assert(nrf_pipe_is_acked(n, 1));
        assert(nrf_pipe_is_acked(n, 0));
    }

    // turn off the other pipes.
    // todo("turn off other pipes\n");

    // check
    for(int i = 2; i < 6; i++)
        assert(!nrf_pipe_is_enabled(n, i));

    // reg=3: NRF_SETUP_AW: setup address size (maybe 1-3 to 3-5)
    uint32_t n_addr_bytes = nrf_get_addr_nbytes(n);
    nrf_put8_chk(n, NRF_SETUP_AW, n_addr_bytes - 2);


    // clear NRF_TX_ADDR for determinism (max length of 5)
    nrf_set_addr(n, NRF_TX_ADDR, 0, n_addr_bytes);

    // set NRF_RX_PW_P1 and  NRF_RX_ADDR_P1
    // set NRF_RX_ADDR_P0 if enabled.
    // could also do when setup pipes.
    nrf_put8_chk(n, NRF_RX_PW_P1, c.nbytes);
    nrf_set_addr(n, NRF_RX_ADDR_P1, n->rxaddr, n_addr_bytes);
    nrf_set_addr(n, NRF_RX_ADDR_P0, n->rxaddr, n_addr_bytes);

    // Set message size = 0 for unused pipes.  
    //  [NOTE: I think redundant, but just to be sure.]
    // ** HARDCODED GODDAMN. Might figure out better way to do this
    nrf_put8_chk(n, NRF_RX_PW_P0, 0);
    nrf_put8_chk(n, NRF_RX_PW_P2, 0);
    nrf_put8_chk(n, NRF_RX_PW_P3, 0);
    nrf_put8_chk(n, NRF_RX_PW_P4, 0);
    nrf_put8_chk(n, NRF_RX_PW_P5, 0);

    // reg=6: RF_SETUP: setup data rate and power.
    // datarate already has the right encoding.
    // 
    // use:
    //  - nrf_default_data_rate;
    //  - nrf_default_db;
    nrf_put8_chk(n, NRF_RF_CH, c.channel);
    val = nrf_default_data_rate | nrf_default_db;
    nrf_put8_chk(n, NRF_RF_SETUP, val);

    // reg=7: status.  p59
    // sanity check that it is empty and nothing else is set.
    //
    // NOTE: if we are in the midst of an active system,
    // it's possible this node receives a message which will
    // change these values.   we might want to set the
    // rx addresses to something that won't get a message.
    // 
    // ideally we would do something where we use different 
    // addresses across reboots?   we get a bit of this benefit
    // by waiting the 100ms.
    uint8_t status_reg = nrf_get8(n, NRF_STATUS); // Wait but RX_P_NO
    assert(status_reg == 0b00001110);

    // i think w/ the nic is off, this better be true.
    assert(!nrf_tx_fifo_full(n));
    assert(nrf_tx_fifo_empty(n));
    assert(!nrf_rx_fifo_full(n));
    assert(nrf_rx_fifo_empty(n));

    assert(!nrf_has_rx_intr(n));
    assert(!nrf_has_tx_intr(n));
    assert(pipeid_empty(nrf_rx_get_pipeid(n)));
    assert(!nrf_rx_has_packet(n));

    // we skip reg=0x8 (observation)
    // we skip reg=0x9 (RPD)
    // we skip reg=0xA (P0)
    // we skip reg=0x10 (TX_ADDR): used only when sending.
    nrf_put8_chk(n, NRF_FEATURE, 0);
    nrf_put8_chk(n, NRF_DYNPD, 0);

    nrf_tx_flush(n);
    nrf_rx_flush(n);

    // p20: go from <PowerDown> to <Standby-I>
    // now go from make sure you delay long enough!
    // todo("go from <PowerDown> to <Standby-I>");
    nrf_put8_chk(n, NRF_CONFIG, nrf_get8(n, NRF_CONFIG) | 0b10);
    delay_ms(100);

    // now go to RX mode: invariant = we are always in RX except for the 
    // small amount of time we need to switch to TX to send a message.
    // todo("go from <Standby-I> to RX");
    nrf_put8_chk(n, NRF_CONFIG, rx_config);
    gpio_write(n->config.ce_pin, 1);
    delay_us(130); // Settling time on state machine 


    // should be true after setup.
    if(acked_p) {
        nrf_opt_assert(n, nrf_get8(n, NRF_CONFIG) == rx_config);
        nrf_opt_assert(n, nrf_pipe_is_enabled(n, 0));
        nrf_opt_assert(n, nrf_pipe_is_enabled(n, 1));
        nrf_opt_assert(n, nrf_pipe_is_acked(n, 0));
        nrf_opt_assert(n, nrf_pipe_is_acked(n, 1));
        nrf_opt_assert(n, nrf_tx_fifo_empty(n));
    } else {
        nrf_opt_assert(n, nrf_get8(n, NRF_CONFIG) == rx_config);
        nrf_opt_assert(n, !nrf_pipe_is_enabled(n, 0));
        nrf_opt_assert(n, nrf_pipe_is_enabled(n, 1));
        nrf_opt_assert(n, !nrf_pipe_is_acked(n, 1));
        nrf_opt_assert(n, nrf_tx_fifo_empty(n));
    }
    return n;
}

// put the device in RX mode: 
//   - go to <Standby-I> so we can transition in 
//     a legal state (*i think*)
// invariant:
//    we are always in RX except for the small amount 
//    of time we need to switch to TX to send a message.
//    we then immediately switch back to RX.
static void nrf_rx_mode(nrf_t *n) {
    todo("go to RX with delay");

    nrf_opt_assert(n, nrf_is_rx(n));
}


// go from RX from TX via the state diagram.
// eliminate the 10usec requirement by going from:
//       RX -> StandbyI (by setting CE=0) 
//          -> TX (by writing data into the FIFO, CE=1 for 10usec)
//          -> Standby-II (CE=1)
//          -> Standby I (CE=0)
// see the state machine on p22, and [p75]
//
// NOTE: If nRF24L01+ is in standby-II mode, it goes to standby-I mode
// immediately if CE is set low. [p75]
static void nrf_tx_mode(nrf_t *n) {
    // NOTE: tx fifo should not be empty.
    todo("go RX->StandbyI->StandbyII->TX");

    nrf_opt_assert(n, nrf_is_tx(n));
}


int nrf_tx_send_ack(nrf_t *n, uint32_t txaddr, 
    const void *msg, unsigned nbytes) {

    // default config for acked state.
    nrf_opt_assert(n, nrf_get8(n, NRF_CONFIG) == rx_config);
    nrf_opt_assert(n, nrf_pipe_is_enabled(n, 0));
    nrf_opt_assert(n, nrf_pipe_is_enabled(n, 1));
    nrf_opt_assert(n, nrf_pipe_is_acked(n, 0));
    nrf_opt_assert(n, nrf_pipe_is_acked(n, 1));
    nrf_opt_assert(n, nrf_tx_fifo_empty(n));

    // if interrupts not enabled: make sure we check for packets.
    while(nrf_get_pkts(n))
        ;

    // TODO: you would implement the rest of the code at this point.
    // see page 75 for transmit.  do the no_ack case first.
    // NOTE: 
    //  - have to set both NRF_RX_ADDR_P0 and NRF_TX_ADDR.
    //    alot more stuff.
    //  - put the data on FIFO before state transitioning.
    //  - if you get more than max_rt_interrupts
    //    you can panic for today.  but dump out the 
    //    configuration: good chance its a bad configure.
    int res = staff_nrf_tx_send_ack(n, txaddr, msg, nbytes);

    // How to increment the total number of retransmissions.
    //  uint8_t cnt = nrf_get8(n, NRF_OBSERVE_TX);
    //  n->tot_retrans  += bits_get(cnt,0,3);

    // when done: tx interrupt better be cleared.
    nrf_opt_assert(n, !nrf_has_tx_intr(n));
    // when done: better be back in rx mode.
    nrf_opt_assert(n, nrf_get8(n, NRF_CONFIG) == rx_config);
    return res;
}

// send packet without hardware ack.
int nrf_tx_send_noack(nrf_t *n, uint32_t txaddr, 
    const void *msg, unsigned nbytes) {

    // default state for no-ack config.
    nrf_opt_assert(n, nrf_get8(n, NRF_CONFIG) == rx_config);
    nrf_opt_assert(n, !nrf_pipe_is_enabled(n, 0));
    nrf_opt_assert(n, nrf_pipe_is_enabled(n, 1));
    nrf_opt_assert(n, !nrf_pipe_is_acked(n, 1));
    nrf_opt_assert(n, nrf_tx_fifo_empty(n));

    // if interrupts not enabled: make sure we check for packets.
    while(nrf_get_pkts(n))
        ;

    // TODO: you would implement the send packet code.
    // see page 75.
    // 1. put packet on TX fifo.
    // 2. put NRF in TX mode, 
    // 3. wait for TX interrupt.
    // 4. assert that tx fifo is empty.
    // 5. Clear the tx interrupt.
    // 6. put back in rx mode.  (via standbyII->standbyI)
    // NOTE: 
    //   - If nRF24L01+ is in standby-II mode, it goes to 
    //     standby-I mode immediately if CE is set low.
    int res = staff_nrf_tx_send_noack(n, txaddr, msg, nbytes);

    // after done: tx interrupt better be cleared.
    nrf_opt_assert(n, !nrf_has_tx_intr(n));
    // after done: better be back in rx mode.
    nrf_opt_assert(n, nrf_get8(n, NRF_CONFIG) == rx_config);
    return res;
}

// while the RX fifo is not empty: read packets into the 
// circular queue <recvq> using <cq_push_n>. 
//
// return number of packets read.
//
// NOTE: 
//   - better be in RX mode!
//   - since we don't have interrupts on, we call this before doing
//     tx to make sure there is space and to reduce chance of dropping
//     packets.
//   - if you send packets you'll have to make sure the packets won't
//     get dropped b/c the receiver is not listening.  can use
//     interrupts.
int nrf_get_pkts(nrf_t *n) {
    // you can't check for packets unless in RX mode.
    nrf_opt_assert(n, nrf_get8(n, NRF_CONFIG) == rx_config);
    if(!nrf_rx_has_packet(n)) 
        return 0; 

    // TODO:
    // data sheet gives the sequence to follow to get packets.
    // p76: 
    // do {
    //    0. assert(pipeid from STATUS != PIPEID_EMPTY)
    //    1. read packet through spi [nrf_getn]
    //    2. push onto the receive queue [cq_push_n],
    //       increment <tot_recv_msgs> and <tot_recv_bytes>
    //    3. clear rx IRQ.
    //    4. read fifo status to see if more packets: 
    //       if so, repeat from (1) --- we need to do this now in case
    //       a packet arrives b/n (1) and (2)
    // } while (rx fifo is not empty)
    int res = staff_nrf_get_pkts(n);

    nrf_opt_assert(n, nrf_get8(n, NRF_CONFIG) == rx_config);
    return res;
}
