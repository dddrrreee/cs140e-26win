#include "rpi.h"
#include "spi.h"

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

    W5500_CHIP_ID =             0x0039;
    W5500_CHIP_ID =             0x0039;
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

    w5500_t chip = w5500_chip_init(0); // 0 IS THE ONE CLOSEST TO IMU

    printk("MOSI: %d, MISO: %d, SCK: %d, CS: %d\n", chip.spi.mosi, chip.spi.miso, chip.spi.clk, chip.spi.chip);

    uint16_t chip_id_reg = 0x0039;

    uint8_t ctrl = W5500_BLOCK_COMMON | SPI_MODE_VDM | W5500_READ;

    dump_spi_regs();

    uint8_t rx[16];
    uint8_t tx[16];

    // Set data length
    tx[0] = (chip_id_reg >> 8) & 0xFF;
    tx[1] = (chip_id_reg) & 0xFF;
    tx[2] = ctrl;
    spi_n_transfer(chip.spi, rx, tx, 4);
    printk("%x\n", rx[3]);

    tx[0] = (chip_id_reg >> 8) & 0xFF;
    tx[1] = (chip_id_reg) & 0xFF;
    tx[2] = ctrl;
    spi_n_transfer(chip.spi, rx, tx, 4);
    printk("%x\n", rx[3]);

    
}
