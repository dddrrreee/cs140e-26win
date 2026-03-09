#include "rpi.h"
#include "spi.h"
#include "../print-utilities.h"

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
        .clk_div = 80,
        .hw_addr = {0x67, 0x67, 0x67, 0x67, 0x67, 0x67},
        .ipv4_addr = {192, 168, 0, 3},
        .gateway_addr = {192, 168, 0, 1},
        .subnet_mask = {255, 255, 255, 0},
        .sockets_enabled = 0b00000001,
        .phy_mode = W5500_ALL_CAPABLE_AUTO_NEG_EN,
    };
    w5500_t nic;

    w5500_init(&nic, &config); // 0 IS THE ONE CLOSEST TO IMU

    printk("MOSI: %d, MISO: %d, SCK: %d, CS: %d\n", nic.spi.mosi, nic.spi.miso, nic.spi.clk, nic.spi.chip);

    const char* message = "Hello World! This is a message that needs to be long enough to send";
    uint16_t msg_len = strlen(message);

    frame_t frame = {
        .dest_hw_addr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        .ethertype = 0x88B5,
    };

    memcpy(frame.src_hw_addr, config.hw_addr, 6); // Needs to go in an init
    memcpy(frame.payload, message, msg_len);
    print_bytes("Frame:", &frame, FRAME_HEADER_BYTES + msg_len);

    frame.ethertype = (frame.ethertype >> 8) | (frame.ethertype << 8); // Swap
    w5500_write_tx_bytes(&nic, &frame, FRAME_HEADER_BYTES + msg_len, W5500_SOCKET_0);
    


    // w5500_put8_chk(&nic, W5500_BLK_COMMON, W5500_REG_MR, 0b1000);

    // w5500_putn(&nic, W5500_BLK_COMMON, W5500_REG_SIPR0, &config.ipv4_addr, 4);
    // uint8_t buf[6];
    // w5500_getn(&nic, W5500_BLK_COMMON, W5500_REG_SIPR0, buf, 4);
    // trace("IP: {%d.%d.%d.%d}\n", buf[0], buf[1], buf[2], buf[3]);

    // w5500_putn(&nic, W5500_BLK_COMMON, W5500_REG_SHAR0, &config.hw_addr, 6);
    // w5500_getn(&nic, W5500_BLK_COMMON, W5500_REG_SHAR0, buf, 6);
    // trace("MAC: {%x:%x:%x:%x:%x:%x}\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

    // w5500_putn_chk(&nic, W5500_BLK_COMMON, W5500_REG_SHAR0, &config.hw_addr, 6);



    // dump_spi_regs();

    // uint8_t rx[16];
    // uint8_t tx[16];

    // // Set nic ID
    // tx[0] = (nic_id_reg >> 8) & 0xFF;
    // tx[1] = (nic_id_reg) & 0xFF;
    // tx[2] = ctrl;
    // spi_n_transfer(nic:spi, rx, tx, 4);
    // printk("%x\n", rx[3]);

    // // 
    // tx[0] = (nic_id_reg >> 8) & 0xFF;
    // tx[1] = (nic_id_reg) & 0xFF;
    // tx[2] = ctrl;
    // spi_n_transfer(nic.spi, rx, tx, 4);
    // printk("%x\n", rx[3]);

    
}
