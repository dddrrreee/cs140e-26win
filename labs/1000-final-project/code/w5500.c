#include "w5500.h"


/**********************************************************
 * Hardware routines that use SPI to read/write 
 * registers
 */

void w5500_init(w5500_t* nic, w5500_conf_t* config) {

    volatile uint8_t val;

    
    // TODO: look in BCM2835 datasheet p. 156 for cluck div
    assert(config->chip_select == 0 || config->chip_select == 1);
    nic->spi = spi_n_init(config->chip_select, config->clk_div);
    assert(nic->spi.chip == config->chip_select);

    // Addresses
    memcpy(nic->hw_addr, config->hw_addr, 6);
    memcpy(nic->ipv4_addr, config->ipv4_addr, 4);
    memcpy(nic->gateway_addr, config->gateway_addr, 4);
    memcpy(nic->subnet_mask, config->subnet_mask, 4);

    cq_init(&nic->recvq, 1);  // initialize the circular queue (forgot errors_fatal_p meaning)

    delay_ms(100); // Time for SPI to start up

    // Check chip id
    uint8_t chip_id = w5500_get8(nic, W5500_BLK_COMMON, W5500_REG_VERSIONR);
    assert(chip_id == W5500_CHIP_ID);

    // Soft reset (p. 32, 42)
    w5500_put8(nic, W5500_BLK_COMMON, W5500_REG_MR, W5500_RESET_REGS);
    while (w5500_get8(nic, W5500_BLK_COMMON, W5500_REG_MR) & W5500_RESET_REGS) {}

    trace("# ----- After reset -----\n");

    // I think.... it needs like 100us? (p. 61) Doing 1ms to be safe
    delay_ms(1);
    w5500_put8(nic, W5500_BLK_COMMON, W5500_REG_PHYCFGR, 0);
    delay_ms(1);

    // Mode register (p. 32-33)
    val = 0;
    // val |= W5500_WOL_EN;         // Don't need to
    val |= W5500_PING_BLOCK_EN;     // Want to implement myself
    // val |= W5500_PPPoE_MODE_EN;  // Only really used by ISPs
    // val |= W5500_FORCE_ARP_EN;   // Want to keep our settings
    w5500_put8_chk(nic, W5500_BLK_COMMON, W5500_REG_MR, val);
    
    // Set addresses (p. 33)
    w5500_putn_chk(nic, W5500_BLK_COMMON, W5500_REG_GAR0, nic->gateway_addr, 4);
    w5500_putn_chk(nic, W5500_BLK_COMMON, W5500_REG_SUBR0, nic->subnet_mask, 4);
    w5500_putn_chk(nic, W5500_BLK_COMMON, W5500_REG_SHAR0, nic->hw_addr, 6);
    w5500_putn_chk(nic, W5500_BLK_COMMON, W5500_REG_SIPR0, nic->ipv4_addr, 4);

    trace("# ----- After addresses -----\n");
    
    // Not doing much with
    // - interrupts (p. 35-37)
    // - retries (p. 38-39)
    // - PPP (p. 39-40)

    // PHY configuration (p. 42)
    val = config->phy_mode; 
    val |= W5500_PHY_MODE_FROM_REG;    // Don't have control over pins
    val |= W5500_RESET_PHY;
    w5500_put8(nic, W5500_BLK_COMMON, W5500_REG_PHYCFGR, val);

    trace("Takes about 3 seconds for it to establish a link. Make sure RJ45 is plugged in");
    delay_ms(3000);
    while (!(w5500_get8(nic, W5500_BLK_COMMON, W5500_REG_PHYCFGR) & W5500_LINK_UP)) {}

    trace("\n# ----- OTHER SOCKET INIT -----\n");

    for (int i = 0; i < 8; i++) {
        uint8_t sock = (i << 5);

        w5500_put8_chk(nic, sock | W5500_BLK_SOCKET_REG,
            W5500_Sn_REG_TXBUF_SIZE, (i == 0) ? 2 : 0);

        w5500_put8_chk(nic, sock | W5500_BLK_SOCKET_REG,
            W5500_Sn_REG_RXBUF_SIZE, (i == 0) ? 2 : 0);
    }

    // uint8_t PHYCFG = w5500_get8(nic, W5500_BLK_COMMON, W5500_REG_PHYCFGR);
    // trace("W5500_REG_PHYCFGR = %x\n", PHYCFG);


    // Initialize SOCKET_0 as MACRAW and set to OPEN
    trace("\n# ----- BEFORE SOCKET 0 INIT -----\n");

    // w5500_put8_chk(nic, W5500_SOCKET_0 | W5500_BLK_SOCKET_REG,
    //             W5500_Sn_REG_TXBUF_SIZE, 2);   // 2 KB

    // w5500_put8_chk(nic, W5500_SOCKET_0 | W5500_BLK_SOCKET_REG,
    //             W5500_Sn_REG_RXBUF_SIZE, 2);   // 2 KB

    w5500_put8_chk(nic, W5500_SOCKET_0 | W5500_BLK_SOCKET_REG,
        W5500_Sn_REG_MR, W5500_MACRAW_PROTOCOL);

    delay_ms(100);


    // uint8_t sr = w5500_get8(nic, W5500_SOCKET_0 | W5500_BLK_SOCKET_REG, W5500_Sn_REG_SR);

    // trace("SR before OPEN = %x\n", sr);  // Should be 0x0

    trace("BEFORE OPEN\n");
    w5500_put8_chk(nic, W5500_SOCKET_0 | W5500_BLK_SOCKET_REG,
        W5500_Sn_REG_CR, W5500_OPEN);
    trace("AFTER OPEN\n");

    uint8_t cr = w5500_get8(nic, W5500_SOCKET_0 | W5500_BLK_SOCKET_REG, W5500_Sn_REG_CR);
    trace("CR after OPEN = %x\n\n\n", cr);
    
    // wait for command to complete
    // while (w5500_get8(nic, W5500_SOCKET_0 | W5500_BLK_SOCKET_REG, W5500_Sn_REG_CR) != 0) {
    while (1) {
        trace("MR: %x\n", w5500_get8(nic,
        W5500_SOCKET_0 | W5500_BLK_SOCKET_REG,
        W5500_Sn_REG_MR));
        trace("SR: %x\n", w5500_get8(nic,
        W5500_SOCKET_0 | W5500_BLK_SOCKET_REG,
        W5500_Sn_REG_SR));
        trace("CR: %x\n", w5500_get8(nic,
        W5500_SOCKET_0 | W5500_BLK_SOCKET_REG,
        W5500_Sn_REG_CR));
        trace("IR: %x\n", w5500_get8(nic,
        W5500_SOCKET_0 | W5500_BLK_SOCKET_REG,
        W5500_Sn_REG_IR));
        trace("PHYCFGR: %x\n\n", w5500_get8(nic, W5500_BLK_COMMON, W5500_REG_PHYCFGR));
        delay_ms(1000);
    }

    trace("# ----- AFTER SOCKET 0 INIT -----\n");


    trace("VERSIONR = %x\n", w5500_get8(nic,W5500_BLK_COMMON,W5500_REG_VERSIONR));
    trace("PHYCFGR  = %x\n", w5500_get8(nic,W5500_BLK_COMMON,W5500_REG_PHYCFGR));
    trace("Sn_SR    = %x\n", w5500_get8(nic,W5500_SOCKET_0|W5500_BLK_SOCKET_REG, W5500_Sn_REG_SR));
}

/**********************************************************
 * Buffers
 */

int w5500_write_tx_bytes(const w5500_t* nic, void* buffer, uint32_t nbytes, uint8_t socket) {

    uint16_t tx_ptr;
    uint8_t ptr_buf[2];

    w5500_getn(nic, socket | W5500_BLK_SOCKET_REG, W5500_Sn_REG_TX_WR0, ptr_buf, 2);
    tx_ptr = (ptr_buf[0] << 8) | ptr_buf[1];

    // TODO: implement buffer length checking. Default value of 2KB for TX buffer size

    // write data to TX buffer
    w5500_putn(nic, socket | W5500_BLK_SOCKET_TX_BUF, tx_ptr, buffer, nbytes);

    // advance pointer
    tx_ptr += nbytes;

    ptr_buf[0] = tx_ptr >> 8;
    ptr_buf[1] = tx_ptr & 0xFF;

    w5500_putn(nic, socket | W5500_BLK_SOCKET_REG, W5500_Sn_REG_TX_WR0, ptr_buf, 2);

    // send command
    w5500_put8(nic, socket | W5500_BLK_SOCKET_REG, W5500_Sn_REG_CR, W5500_SEND_MAC);

    trace("SR reg: %x\n", w5500_get8(nic, socket | W5500_BLK_SOCKET_REG, W5500_Sn_REG_SR));
    
    // while (!(w5500_get8(nic, W5500_SOCKET_0|W5500_BLK_SOCKET_REG, W5500_Sn_REG_IR) & 0x10))
    //     ;

    return nbytes;
}


/**********************************************************
 * Hardware routines that use SPI to read/write 
 * registers
 */

uint8_t w5500_get8(const w5500_t* nic, uint8_t block, uint16_t reg) {
    uint8_t rx[4], tx[4];

    tx[0] = reg >> 8;
    tx[1] = reg & 0xFF;
    tx[2] = W5500_READ | SPI_MODE_VDM | block;
    tx[3] = 0;

    trace("TX: {%x, %x, %x, %x}\n", tx[0], tx[1], tx[2], tx[3]);
    spi_n_transfer(nic->spi, rx,tx,4);
    trace("Received {%x}\n", rx[3]);

    // trace("wr8: sent: tx[0]=%b, tx[1]=%b\n", tx[0], tx[1]);
    // spi_n_transfer(nic->spi, rx,tx,4);
    // trace("wr8: recv: rx[0]=%b, rx[1]=%b\n", rx[0], rx[1]);
    
    return rx[3];
}

uint8_t w5500_put8(const w5500_t* nic, uint8_t block, uint16_t reg, uint8_t val) {
    uint8_t rx[4], tx[4];

    tx[0] = reg >> 8;
    tx[1] = reg & 0xFF;
    tx[2] = W5500_WRITE | SPI_MODE_VDM | block;
    tx[3] = val;

    trace("TX: {%x, %x, %x, %x}\n", tx[0], tx[1], tx[2], tx[3]);
    spi_n_transfer(nic->spi, rx,tx,4);
    // trace("Received {%x}\n", rx[3]);
    
    return rx[0]; // Status
}

uint8_t w5500_put8_chk_helper(src_loc_t l, const w5500_t* nic, uint8_t block, uint16_t reg, uint8_t val) {
    uint8_t status = w5500_put8(nic, block, reg, val);

    // doing this *always* ran into problems during message sending.
    // so made this seperate routine.
    uint8_t x = w5500_get8(nic, block, reg);
    if(x != val)
        loc_panic(l, "reg=<%x>: got <%x>, expected <%x>\n", reg, x, val);
    return status;
}

uint8_t w5500_getn(const w5500_t* nic, uint8_t block, uint16_t reg, void *data, uint32_t nbytes) {
    uint8_t rx[W5500_MAX_RW_BUF_SIZE], tx[W5500_MAX_RW_BUF_SIZE];
    assert(nbytes + 3 < sizeof(rx));

    tx[0] = reg >> 8;
    tx[1] = reg & 0xFF;
    tx[2] = W5500_READ | SPI_MODE_VDM | block;

    spi_n_transfer(nic->spi, rx, tx, nbytes + 3);
    memcpy(data, &rx[3], nbytes);

    return rx[0];   // status
}

uint8_t w5500_putn(const w5500_t* nic, uint8_t block, uint16_t reg, void *data, uint32_t nbytes) {
    uint8_t rx[W5500_MAX_RW_BUF_SIZE], tx[W5500_MAX_RW_BUF_SIZE];
    assert(nbytes + 3 < sizeof(tx));

    tx[0] = reg >> 8;
    tx[1] = reg & 0xFF;
    tx[2] = W5500_WRITE | SPI_MODE_VDM | block;

    memcpy(&tx[3], data, nbytes);
    spi_n_transfer(nic->spi, rx, tx, nbytes + 3);

    return rx[0];   // status
}

uint8_t w5500_putn_chk_helper(
    src_loc_t l, const w5500_t* nic, uint8_t block, uint16_t reg, void *bytes, uint32_t nbytes) {
    uint8_t status = w5500_putn(nic, block, reg, bytes, nbytes);

    uint8_t rx[W5500_MAX_RW_BUF_SIZE];
    memcpy(rx, bytes, nbytes);

    w5500_getn(nic, block, reg, rx, nbytes);

    // Checker part
    int valid = 1;
    for(uint32_t i = 0; i < nbytes; i++) {

        if(((uint8_t*)bytes)[i] != rx[i]) {
            loc_debug(l, "putn_check reg mismatch at offset %u: got %x, expected %x\n", 
                    i, rx[i], ((uint8_t*)bytes)[i]);
            valid = 0;
        }
    }
    if (!valid)
        loc_panic_fn(l, "\n");


    return status;
}

/**********************************************************
 * other routines
 */