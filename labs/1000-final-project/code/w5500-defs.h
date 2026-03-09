#ifndef __W5500_DEFS_H__
#define __W5500_DEFS_H__
#include "rpi.h"
#include "libc/bit-support.h"


#define W5500_CHIP_ID 4
#define W5500_MAX_RW_BUF_SIZE 255

/* ---------- Frame/Packet Constants ---------- */
enum {
    MAX_FRAME_PAYLOAD_SIZE = 100, // Just doing this for now (usually 1500)
    FRAME_HEADER_BYTES = 14, // 6 bytes dest hw addr, 6 bytes src hw addr, 2 bytes length/length
    MAX = FRAME_HEADER_BYTES + MAX_FRAME_PAYLOAD_SIZE,
};

/* ---------- Control Phase (p. 15-16) ---------- */
enum {
    W5500_BLK_COMMON            = 0b00000 << 3, // Doesn't really mean anything but just symbolic

    W5500_SOCKET_0              = 0b000 << 5, // Use with REG/TX_BUF/RX_BUF
    W5500_SOCKET_1              = 0b001 << 5, // Use with REG/TX_BUF/RX_BUF
    W5500_SOCKET_2              = 0b010 << 5, // Use with REG/TX_BUF/RX_BUF
    W5500_SOCKET_3              = 0b011 << 5, // Use with REG/TX_BUF/RX_BUF
    W5500_SOCKET_4              = 0b100 << 5, // Use with REG/TX_BUF/RX_BUF
    W5500_SOCKET_5              = 0b101 << 5, // Use with REG/TX_BUF/RX_BUF
    W5500_SOCKET_6              = 0b110 << 5, // Use with REG/TX_BUF/RX_BUF
    W5500_SOCKET_7              = 0b111 << 5, // Use with REG/TX_BUF/RX_BUF

    W5500_Sn_TX_BUF                = 0b10 << 3,
    W5500_Sn_RX_BUF                = 0b11 << 3,

    W5500_BLK_SOCKET_REG        = 0b01 << 3,
    W5500_BLK_SOCKET_TX_BUF     = 0b10 << 3,
    W5500_BLK_SOCKET_RX_BUF     = 0b11 << 3,

    W5500_READ                  = 0 << 2,
    W5500_WRITE                 = 1 << 2,

    SPI_MODE_VDM                = 0b00,
};

/* ---------- Common Register Block (p. 29, 32-43) ---------- */
enum {
    W5500_REG_MR                = 0x0000, // Mode

    W5500_REG_GAR0              = 0x0001, // Gateway IP (4 bytes)
    W5500_REG_SUBR0             = 0x0005, // Subnet Mask (4 bytes)
    W5500_REG_SHAR0             = 0x0009, // Hardware MAC Address (6 bytes)
    W5500_REG_SIPR0             = 0x000F, // IP Address (4 bytes)

    W5500_REG_SIR               = 0x0017, // Socket Interrupt
    W5500_REG_SIMR              = 0x0018, // Socket Interrupt Mask
    
    W5500_REG_PHYCFGR           = 0x002E, // PHY configuration

    W5500_REG_VERSIONR          = 0x0039, // CHIP ID
};

/* ---------- Socket Register Block (p. 30, 44-58) ---------- */
enum {
    W5500_Sn_REG_MR             = 0x0000, // Socket Mode
    W5500_Sn_REG_CR             = 0x0001, // Command Register
    W5500_Sn_REG_IR             = 0x0002, // Interrupt Register
    W5500_Sn_REG_SR             = 0x0003, // Status Register

    W5500_Sn_REG_DHAR0          = 0x0006, // Socket Destination Hardware Address (6 bytes)
    W5500_Sn_REG_DIPR0          = 0x000C, // Socket Destination IP Address (4 bytes)
    W5500_Sn_REG_DPORT0         = 0x0010, // Socket Destination Port (2 bytes)

    W5500_Sn_REG_TTL            = 0x0016, // Socket Time-to-live

    W5500_Sn_REG_RXBUF_SIZE     = 0x001E, // Socket Receive Buffer Size
    W5500_Sn_REG_TXBUF_SIZE     = 0x001F, // Socket Transmit Buffer Size

    W5500_Sn_REG_TX_FSR0        = 0x0020, // Socket TX Buffer Free Size (2 bytes)
    W5500_Sn_REG_TX_RD0         = 0x0022, // Socket TX Buffer Read Pointer (2 bytes)
    W5500_Sn_REG_TX_WR0         = 0x0024, // Socket TX Buffer Write Pointer (2 bytes)

    W5500_Sn_REG_RX_FSR0        = 0x0026, // Socket RX Buffer Free Size (2 bytes)
    W5500_Sn_REG_RX_RD0         = 0x0028, // Socket RX Buffer Read Pointer (2 bytes)
    W5500_Sn_REG_RX_WR0         = 0x002A, // Socket RX Buffer Write Pointer (2 bytes)

    W5500_Sn_REG_IMR            = 0x002C, // Socket Interrupt Mask
};

/* ---------- Socket Commands Register (p. 46-47) ---------- */

enum {
    W5500_OPEN                  = 0x01,
    W5500_LISTEN                = 0x02,
    W5500_CONNECT               = 0x04,
    W5500_DISCON                = 0x08,
    W5500_CLOSE                 = 0x10,
    W5500_SEND                  = 0x20,
    W5500_SEND_MAC              = 0x21,
    W5500_SEND_KEEP             = 0x22,
    W5500_RECV                  = 0x40,
};

/* ---------- Mode Register (p. 32-33) ---------- */
enum {
    W5500_RESET_REGS            = 1 << 7,

    W5500_WOL_EN                = 1 << 5, // Pulls INTn pin low if WOL packet received
    W5500_WOL_DIS               = 0 << 5, // Doesn't pull INTn pin low if WOL packet received

    W5500_PING_BLOCK_EN         = 1 << 4, // Blocks PING requests
    W5500_PING_BLOCK_DIS        = 0 << 4, // Allows PING requests

    // Point-to-Point protocol 
    // (https://en.wikipedia.org/wiki/Point-to-Point_Protocol_over_Ethernet)
    // Mostly used for ISPs apparently
    W5500_PPPoE_MODE_EN         = 1 << 3, 
    W5500_PPPoE_MODE_DIS        = 0 << 3, 

    W5500_FORCE_ARP_EN          = 1 << 1, // Send ARP IMMEDIATELY on startup
    W5500_FORCE_ARP_DIS         = 0 << 1, // Do not send ARP on startup
};

/* ---------- PHY Config Register (p. 32-33) ---------- */
enum W5500 {
    W5500_RESET_PHY             = 1 << 7,

    W5500_PHY_MODE_FROM_REG     = 1 << 6, // PHY mode is set from bits in this register
    W5500_PHY_MODE_FROM_PINS    = 0 << 6, // PHY mode is set from hardware pins

    W5500_10BT_HALF_DUP_AUTO_NEG_DIS  // PHY Mode
        = 0b000 << 3,
    W5500_10BT_FULL_DUP_AUTO_NEG_DIS  // PHY Mode
        = 0b001 << 3,
    W5500_100BT_HALF_DUP_AUTO_NEG_DIS // PHY Mode
        = 0b010 << 3,
    W5500_100BT_FULL_DUP_AUTO_NEG_DIS // PHY Mode
        = 0b011 << 3,
    W5500_100BT_HALF_DUP_AUTO_NEG_EN  // PHY Mode
        = 0b100 << 3,
    W5500_ALL_CAPABLE_AUTO_NEG_EN      // PHY Mode
        = 0b111 << 3,

    W5500_FULL_DUPLEX           = 1, // From Read-only bits
    W5500_HALF_DUPLEX           = 0, // From Read-only bits

    W5500_100_Mbps              = 1, // From Read-only bits
    W5500_10_Mbps               = 0, // From Read-only bits

    W5500_LINK_UP               = 1, // From Read-only bits
    W5500_LINK_DOWN             = 0, // From Read-only bits
};


/* ---------- Socket Mode Register (p. 32-33) ---------- */
enum {
    W5500_MACRAW_PROTOCOL       = 0b0100,
};


#endif
