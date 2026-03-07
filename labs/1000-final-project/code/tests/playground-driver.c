#include "rpi.h"
#include "spi.h"

#include "../w5500.h"
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
        printk("SPI reg %x: %x\n", i, GET32(i));
    }
}


void notmain(void) { 

    dump_spi_regs();

    w5500_conf_t config = {
        .chip_select = 0,
        .clk_div = 40,
        .hw_addr = {0x67, 0x67, 0x67, 0x67, 0x67, 0x67},
        .ipv4_addr = {192, 168, 0, 3},
        .gateway_addr = {192, 168, 0, 1},
        .subnet_mask = {255, 255, 255, 0},
        .sockets_enabled = 0b00000001,
        .phy_mode = W5500_ALL_CAPBLE_AUTO_NEG_EN,
    };
    w5500_t chip;

    w5500_init(&chip, &config); // 0 IS THE ONE CLOSEST TO IMU

    printk("MOSI: %d, MISO: %d, SCK: %d, CS: %d\n", chip.spi.mosi, chip.spi.miso, chip.spi.clk, chip.spi.chip);

    // w5500_put8_chk(&chip, W5500_BLK_COMMON, W5500_REG_MR, 0b1000);

    // w5500_putn(&chip, W5500_BLK_COMMON, W5500_REG_SIPR0, &config.ipv4_addr, 4);
    // uint8_t buf[6];
    // w5500_getn(&chip, W5500_BLK_COMMON, W5500_REG_SIPR0, buf, 4);
    // trace("IP: {%d.%d.%d.%d}\n", buf[0], buf[1], buf[2], buf[3]);

    // w5500_putn(&chip, W5500_BLK_COMMON, W5500_REG_SHAR0, &config.hw_addr, 6);
    // w5500_getn(&chip, W5500_BLK_COMMON, W5500_REG_SHAR0, buf, 6);
    // trace("MAC: {%x:%x:%x:%x:%x:%x}\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

    // w5500_putn_chk(&chip, W5500_BLK_COMMON, W5500_REG_SHAR0, &config.hw_addr, 6);



    // dump_spi_regs();

    // uint8_t rx[16];
    // uint8_t tx[16];

    // // Set chip ID
    // tx[0] = (chip_id_reg >> 8) & 0xFF;
    // tx[1] = (chip_id_reg) & 0xFF;
    // tx[2] = ctrl;
    // spi_n_transfer(chip:spi, rx, tx, 4);
    // printk("%x\n", rx[3]);

    // // 
    // tx[0] = (chip_id_reg >> 8) & 0xFF;
    // tx[1] = (chip_id_reg) & 0xFF;
    // tx[2] = ctrl;
    // spi_n_transfer(chip.spi, rx, tx, 4);
    // printk("%x\n", rx[3]);

    
}
