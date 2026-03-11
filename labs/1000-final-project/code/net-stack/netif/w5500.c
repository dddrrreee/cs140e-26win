#include "w5500.h"

#include "../../endian.h"
#include "../../crc-16.h"
#include "../../print-utilities.h"

// TODO: ADD PAGE NUMEBRS FOR ALL OF THESE

#define BUFFER_SIZE_KB 2
#define RX_BUFFER_SIZE_BYTES (BUFFER_SIZE_KB * 1024)
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
    memcpy(nic->hw_addr, config->hw_addr, MAC_ADDR_LENGTH);
    memcpy(nic->ipv4_addr, config->ipv4_addr, IPV4_ADDR_LENGTH);
    memcpy(nic->gateway_addr, config->gateway_addr, IPV4_ADDR_LENGTH);
    memcpy(nic->subnet_mask, config->subnet_mask, IPV4_ADDR_LENGTH);

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
    w5500_putn_chk(nic, W5500_BLK_COMMON, W5500_REG_GAR0, nic->gateway_addr, IPV4_ADDR_LENGTH);
    w5500_putn_chk(nic, W5500_BLK_COMMON, W5500_REG_SUBR0, nic->subnet_mask, IPV4_ADDR_LENGTH);
    w5500_putn_chk(nic, W5500_BLK_COMMON, W5500_REG_SHAR0, nic->hw_addr, MAC_ADDR_LENGTH);
    w5500_putn_chk(nic, W5500_BLK_COMMON, W5500_REG_SIPR0, nic->ipv4_addr, IPV4_ADDR_LENGTH);

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

    trace("\n# ----- OTHER SOCKET INIT -----\n");
    // TODO: Turn other sockets off

    // Clear buffers
    for (int i = 0; i < 8; i++) {
        uint8_t sock = (i << 5);

        w5500_put8_chk(nic, sock | W5500_BLK_SOCKET_REG,
            W5500_Sn_REG_TXBUF_SIZE, (i == 0) ? BUFFER_SIZE_KB : 0);

        w5500_put8_chk(nic, sock | W5500_BLK_SOCKET_REG,
            W5500_Sn_REG_RXBUF_SIZE, (i == 0) ? BUFFER_SIZE_KB : 0);
    }

    
    trace("\n# ----- SOCKET 0 INIT -----\n");

    // Close and Open Socket 0 as MACRAW
    w5500_put8_chk(nic, W5500_SOCKET_0 | W5500_BLK_SOCKET_REG,
        W5500_Sn_REG_MR, W5500_MACRAW_PROTOCOL);

    w5500_put8(nic, W5500_SOCKET_0 | W5500_BLK_SOCKET_REG,
        W5500_Sn_REG_CR, W5500_CLOSE);

    w5500_put8(nic, W5500_SOCKET_0 | W5500_BLK_SOCKET_REG,
        W5500_Sn_REG_CR, W5500_OPEN);

    // Make sure it was put in MACRAW mode correctly (MUST DO AFTER OPENING IT)
    // "It changes to SOCK_MACRAW when S0_MR(P[3:0] = ‘0100’ and OPEN command is ordered" (p. 50)
    assert(w5500_get8(nic,
        W5500_SOCKET_0 | W5500_BLK_SOCKET_REG,
        W5500_Sn_REG_SR) == W5500_SOCK_MACRAW);

    w5500_fast_flush_rx(nic, W5500_SOCKET_0); // Clear RX buffer

    trace("Takes about 3 seconds for it to establish a link. Make sure RJ45 is plugged in\n");
    while(!(w5500_get8(nic, W5500_BLK_COMMON, W5500_REG_PHYCFGR) & 1)) {}
    trace("Connected!\n");


    trace("VERSIONR = %x\n", w5500_get8(nic,W5500_BLK_COMMON,W5500_REG_VERSIONR));
    trace("PHYCFGR  = %x\n", w5500_get8(nic,W5500_BLK_COMMON,W5500_REG_PHYCFGR));
    trace("Sn_SR    = %x\n", w5500_get8(nic,W5500_SOCKET_0|W5500_BLK_SOCKET_REG, W5500_Sn_REG_SR));
}

/**********************************************************
 * Buffers
 */

uint16_t w5500_tx_available(const w5500_t* nic, uint8_t socket) {
    uint8_t buf[2];
    w5500_getn(nic, socket | W5500_BLK_SOCKET_REG, W5500_Sn_REG_TX_FSR0, buf, 2);
    return (buf[0] << 8) | buf[1];
}

uint16_t w5500_write_tx_bytes(const w5500_t* nic, const void* buffer, uint32_t nbytes, uint8_t socket) {

    uint16_t tx_ptr;
    uint8_t ptr_buf[2];

    // ---------- Get TX buffer pointer ---------- 
    w5500_getn(nic, socket | W5500_BLK_SOCKET_REG, W5500_Sn_REG_TX_WR0, ptr_buf, 2);
    tx_ptr = (ptr_buf[0] << 8) | ptr_buf[1];
    
    // TODO: implement buffer length checking. Default value of 2KB for TX buffer size
    
    // ---------- Advance TX buffer pointer ---------- 
    // TODO: wrap / mod
    tx_ptr += nbytes;
    ptr_buf[0] = tx_ptr >> 8;
    ptr_buf[1] = tx_ptr & 0xFF;
    

    // ---------- write data ----------
    w5500_putn(nic, socket | W5500_BLK_SOCKET_TX_BUF, tx_ptr, buffer, nbytes);

    // ---------- update ptr and send ---------- 
    w5500_putn(nic, socket | W5500_BLK_SOCKET_REG, W5500_Sn_REG_TX_WR0, ptr_buf, 2);
    w5500_put8(nic, socket | W5500_BLK_SOCKET_REG, W5500_Sn_REG_CR, W5500_SEND_MAC);

    return nbytes;
}

uint16_t w5500_rx_available(const w5500_t* nic, uint8_t socket) {
    uint8_t buf[2];
    w5500_getn(nic, socket | W5500_BLK_SOCKET_REG, W5500_Sn_REG_RX_RSR0, buf, 2);

    uint16_t bytes_available = (buf[0] << 8) | buf[1];

    if (bytes_available > BUFFER_SIZE_KB * 1024 || bytes_available == 0xFFFF) {  // Invalid if > buffer size or all 1s
        return 0;
    }

    return bytes_available;
}

// Helper: Advance `uint16_t* rx_ptr` AND CONFIRM BY SENDING RECV COMMAND
static void w5500_advance_rx_pointer(const w5500_t* nic, uint8_t socket, uint16_t* rx_ptr, uint16_t data_len) {
    uint8_t ptr_buf[2];
    *rx_ptr += data_len;
    *rx_ptr %= RX_BUFFER_SIZE_BYTES;
    ptr_buf[0] = *rx_ptr >> 8;
    ptr_buf[1] = *rx_ptr & 0xFF;
    w5500_putn(nic, socket | W5500_BLK_SOCKET_REG, W5500_Sn_REG_RX_RD0, ptr_buf, 2);
    w5500_put8(nic, socket | W5500_BLK_SOCKET_REG, W5500_Sn_REG_CR, W5500_RECV);
}

// Helper: Read frame data with circular buffer wrap AND ADVANCE `uint16_t* rx_ptr`
static void w5500_read_frame_data(const w5500_t* nic, uint8_t socket, uint16_t* rx_ptr, void* buffer, uint16_t data_len) {
    uint16_t bytes_to_end = RX_BUFFER_SIZE_BYTES - *rx_ptr;
    if (data_len <= bytes_to_end) {
        w5500_getn(nic, socket | W5500_BLK_SOCKET_RX_BUF, *rx_ptr, buffer, data_len);
    } else {
        w5500_getn(nic, socket | W5500_BLK_SOCKET_RX_BUF, *rx_ptr, buffer, bytes_to_end);
        w5500_getn(nic, socket | W5500_BLK_SOCKET_RX_BUF, 0, (uint8_t*)buffer + bytes_to_end, data_len - bytes_to_end);
    }
    w5500_advance_rx_pointer(nic, socket, rx_ptr, data_len);
}

// Helper: Read 2-byte length header AND ADVANCE `uint16_t* rx_ptr`
static uint16_t w5500_read_length_header(const w5500_t* nic, uint8_t socket, uint16_t* rx_ptr) {
    uint8_t head[2];
    uint8_t ptr_buf[2];

    // Read length header and advance pointer by 2
    w5500_getn(nic, socket | W5500_BLK_SOCKET_RX_BUF, *rx_ptr, head, 2);
    w5500_advance_rx_pointer(nic, socket, rx_ptr, 2);

    return (head[0] << 8) | head[1];
}

uint16_t w5500_read_rx_bytes(const w5500_t* nic, void* buffer, uint8_t socket) {
    uint16_t avail = w5500_rx_available(nic, socket);
    if (avail == 0) return 0;

    uint16_t rx_ptr;
    uint8_t ptr_buf[2];

    // Get current RX read pointer
    w5500_getn(nic, socket | W5500_BLK_SOCKET_REG, W5500_Sn_REG_RX_RD0, ptr_buf, 2);
    rx_ptr = (ptr_buf[0] << 8) | ptr_buf[1];

    // Read length header (which advances pointer)
    uint16_t raw_len = w5500_read_length_header(nic, socket, &rx_ptr);
    uint16_t data_len = raw_len - 2;  // Subtract header size

    // Simple frame size checking
    // should this be in here or in inet_frame_handler?
    if (data_len > FRAME_MAX_SIZE) {
        // Ignore oversized frame by skipping it
        w5500_advance_rx_pointer(nic, socket, &rx_ptr, data_len);
        return 0;
    }

    // Read frame data (and advance pointer)
    uint16_t data_start = rx_ptr % RX_BUFFER_SIZE_BYTES;
    w5500_read_frame_data(nic, socket, &rx_ptr, buffer, data_len);

    return data_len;
}

void w5500_fast_flush_rx(const w5500_t* nic, uint8_t socket)
{
    uint16_t rx_ptr;
    uint8_t buf[2];

    // ---------- Read RX write pointer ---------- 
    w5500_getn(nic, socket | W5500_BLK_SOCKET_REG, W5500_Sn_REG_RX_WR0, buf, 2);
    rx_ptr = (buf[0] << 8) | buf[1];

    // ---------- Set RX read pointer = write pointer ---------- 
    buf[0] = rx_ptr >> 8;
    buf[1] = rx_ptr & 0xFF;
    w5500_putn(nic, socket | W5500_BLK_SOCKET_REG, W5500_Sn_REG_RX_RD0, buf, 2);

    // ---------- Tell chip RX data was consumed ---------- 
    w5500_put8(nic, socket | W5500_BLK_SOCKET_REG, W5500_Sn_REG_CR, W5500_RECV);
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

    // trace("TX: {%x, %x, %x, %x}\n", tx[0], tx[1], tx[2], tx[3]);
    spi_n_transfer(nic->spi, rx,tx,IPV4_ADDR_LENGTH);
    // trace("Received {%x}\n", rx[3]);

    // trace("wr8: sent: tx[0]=%b, tx[1]=%b\n", tx[0], tx[1]);
    // spi_n_transfer(nic->spi, rx,tx,IPV4_ADDR_LENGTH);
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
    spi_n_transfer(nic->spi, rx,tx,IPV4_ADDR_LENGTH);
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

uint8_t w5500_getn(const w5500_t* nic, uint8_t block, uint16_t reg, void *bytes, uint32_t nbytes) {
    uint8_t rx[W5500_MAX_RW_BUF_SIZE], tx[W5500_MAX_RW_BUF_SIZE];
    if (nbytes + 3 >= sizeof(rx))
        panic("nbytes (%u + 3) too large for getn (%u)\n", nbytes, sizeof(rx));

    tx[0] = reg >> 8;
    tx[1] = reg & 0xFF;
    tx[2] = W5500_READ | SPI_MODE_VDM | block;

    spi_n_transfer(nic->spi, rx, tx, nbytes + 3);
    memcpy(bytes, &rx[3], nbytes);

    return rx[0];   // status
}

uint8_t w5500_putn(const w5500_t* nic, uint8_t block, uint16_t reg, const void *bytes, uint32_t nbytes) {
    uint8_t rx[W5500_MAX_RW_BUF_SIZE], tx[W5500_MAX_RW_BUF_SIZE];
    assert(nbytes + 3 < sizeof(tx));

    tx[0] = reg >> 8;
    tx[1] = reg & 0xFF;
    tx[2] = W5500_WRITE | SPI_MODE_VDM | block;

    memcpy(&tx[3], bytes, nbytes);
    spi_n_transfer(nic->spi, rx, tx, nbytes + 3);

    return rx[0];   // status
}

uint8_t w5500_putn_chk_helper(
    src_loc_t l, const w5500_t* nic, uint8_t block, uint16_t reg, const void *bytes, uint32_t nbytes) {
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