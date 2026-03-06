#include "rpi.h"
#include "spi.h"

// typedef struct {
//     unsigned chip;
//     unsigned div;

//     unsigned mosi,miso,clk,ce;
// } spi_t;

typedef struct {
    spi_t spi;
    uint32_t cs;
} w5500_t;

w5500_t w5500_chip_init(unsigned cs) {
    // p 20 implies takes 100ms to get spiice online?
    delay_ms(100);
    assert(cs == 0 || cs == 1);

    // the larger you make the clock divider, the slower 
    // SPI goes.  
    //
    // NOTE: the NRF has a limit [look in the datasheet]
    spi_t s = spi_n_init(cs, 40 /* 1024 */);
    assert(s.chip == cs);

    // give spi time to start-up [not sure if needed]
    delay_ms(100);

    // w5500_t dev = {.spi = spi, .cs = cs};

    return (w5500_t){.spi = s, .cs = cs};

}

enum {
    W5500_READ =                0,
    W5500_WRITE =               1 << 2,

    SPI_MODE_VDM =              0,
    
    W5500_BLOCK_COMMON =        0b00000 << 3,
    W5500_BLOCK_SOCK_0 =        0b00001 << 3,
    W5500_BLOCK_SOCK_TX_0 =     0b00010 << 3,
    W5500_BLOCK_SOCK_RX_0 =     0b00100 << 3,
};

enum {
    SPI_REG_BASE = 0x20204000,
    SPI_REG_CS = (SPI_REG_BASE + 0),
    SPI_REG_FIFO = (SPI_REG_BASE + 0x4),
    SPI_REG_CLK = (SPI_REG_BASE + 0x8),
    SPI_REG_DLEN = (SPI_REG_BASE + 0xC),
    SPI_REG_LTOH = (SPI_REG_BASE + 0x10),
    SPI_REG_DC = (SPI_REG_BASE + 0x14)
};

void dump_spi_regs() {
    for (uint32_t i = SPI_REG_BASE; i <= SPI_REG_DC; i += 4) {
        printk("SPI reg %d: %x\n", i, GET32(i));
    }
}


void notmain(void) { 

    dump_spi_regs();

    w5500_t chip = w5500_chip_init(1);

    printk("MOSI: %d, MISO: %d, SCK: %d, CS: %d\n", chip.spi.mosi, chip.spi.miso, chip.spi.clk, chip.spi.chip);

    uint16_t chip_id_reg = 0x0039;

    uint8_t ctrl = W5500_BLOCK_COMMON | SPI_MODE_VDM | W5500_READ;

    dump_spi_regs();

    uint8_t rx[8];
    uint8_t tx[8];

    // Set data length
    tx[0] = 0b00100000 | 3;
    tx[1] = 3;
    spi_n_transfer(chip.spi, rx, tx, 2);

    // Read to make sure it worked
    tx[0] = 3;
    spi_n_transfer(chip.spi, rx, tx, 2);
    printk("%x\n", rx[1]);

    memset(rx, 0, 6);
    tx[0] = 0x0A;
    spi_n_transfer(chip.spi, rx, tx, 6);
    printk("%x %x %x %x %x\n", rx[1], rx[2], rx[3], rx[4], rx[5]);
    // printk("%x\n", *(uint16_t*)rx);

    // uint8_t tx[4] = {
    //     (chip_id_reg >> 8) & 0xFF,
    //     (chip_id_reg >> 8) & 0xFF,
    //     ctrl, 0};

    // uint8_t rx[4];
    // memset(rx, 0, 4);

    // spi_n_transfer(chip.spi, rx, tx, 4);
    
    // printk("Wow this works!\n");
}
