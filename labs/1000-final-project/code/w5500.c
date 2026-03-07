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
    // I think.... it needs like 100us? (p. 61) Doing 1ms to be safe
    delay_ms(1);
    w5500_put8(nic, W5500_BLK_COMMON, W5500_REG_PHYCFGR, W5500_RESET_PHY);
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
    
    // Not doing much with
    // - interrupts (p. 35-37)
    // - retries (p. 38-39)
    // - PPP (p. 39-40)

    // PHY configuration (p. 42)
    // val = 0;
    // // val |= W5500_WOL_EN;         // Don't need to
    // val |= W5500_PING_BLOCK_EN;     // Want to implement myself
    // // val |= W5500_PPPoE_MODE_EN;  // Only really used by ISPs
    // // val |= W5500_FORCE_ARP_EN;   // Want to keep our settings
    // w5500_put8_chk(nic, W5500_BLK_COMMON, W5500_REG_MR, val);

}

uint8_t w5500_get8(const w5500_t* nic, uint8_t block, uint16_t reg) {
    uint8_t rx[4], tx[4];

    tx[0] = reg >> 8;
    tx[1] = reg & 0xFF;
    tx[2] = W5500_READ | SPI_MODE_VDM | block;
    tx[3] = 0;

    // trace("TX: {%x, %x, %x, %x}\n", tx[0], tx[1], tx[2], tx[3]);
    spi_n_transfer(nic->spi, rx,tx,4);
    // trace("Received {%x}\n", rx[3]);

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

    // trace("TX: {%x, %x, %x, %x}\n", tx[0], tx[1], tx[2], tx[3]);
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

