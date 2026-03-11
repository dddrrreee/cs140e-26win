// Layer 2 (high-ish)

#include "arp.h"

#include "data-link.h" // For setting mac
#include "network.h" // For setting ip

#include "../print-utilities.h"
#include "../endian.h"

static arp_table_entry_t _arp_table[ARP_TABLE_SIZE];

static int verbose_p = 0;

void arp_set_verbose_p(int verbose) {verbose_p = verbose;}

/**********************************************************
 * Public interface
 */

void inet_clear_arp_table() {
    memset(_arp_table, 0, sizeof(_arp_table)); // Makes everything 0 and invalid
}


int find_arp_entry_by_ipv4(const uint8_t* ipv4_addr, uint32_t* table_index) {

    int found_empty = 0;
    *table_index = ARP_TABLE_SIZE; // Place that is not in the table as the default in case table is full
    for (uint32_t i = 0; i < ARP_TABLE_SIZE; i++) {

        if (memcmp(_arp_table[i].ip_addr, ipv4_addr, IPV4_ADDR_LENGTH) == 0 ) {
            *table_index = i;
            if (!_arp_table[i].valid)
                return INET_ARP_FOUND_BUT_INVALID;
            return INET_SUCCESS;
        }

        // First 
        if (!found_empty && !_arp_table[i].valid) {
            found_empty = 1;
            *table_index = i;
        }
    }
    return INET_ARP_NO_TABLE_ENTRY;
}

int find_arp_entry_by_mac(const uint8_t* hw_addr, uint32_t* table_index) {

    int found_empty = 0;
    *table_index = ARP_TABLE_SIZE; // Place that is not in the table as the default in case table is full
    for (uint32_t i = 0; i < ARP_TABLE_SIZE; i++) {

        if (memcmp(_arp_table[i].hw_addr, hw_addr, MAC_ADDR_LENGTH) == 0 ) {
            *table_index = i;
            if (!_arp_table[i].valid)
                return INET_ARP_FOUND_BUT_INVALID;
            return INET_SUCCESS;
        }
        // First 
        if (!found_empty && !_arp_table[i].valid) {
            found_empty = 1;
            *table_index = i;
        }
    }
    return INET_ARP_NO_TABLE_ENTRY;
}

// TODO Invalidate entry 

int inet_add_arp_entry(const uint8_t* their_ipv4_addr, const uint8_t* their_hw_addr) {
    uint32_t idx;
    int err = find_arp_entry_by_ipv4(their_ipv4_addr, &idx);

    switch (err) {
        case INET_ARP_NO_TABLE_ENTRY:
        case INET_SUCCESS:
            memcpy(_arp_table[idx].hw_addr, their_hw_addr, MAC_ADDR_LENGTH);
            memcpy(_arp_table[idx].ip_addr, their_ipv4_addr, IPV4_ADDR_LENGTH);
            _arp_table[idx].valid = 1;
            if (verbose_p)
                trace_arp_table("Arp table now");
            return INET_SUCCESS;

        case INET_ARP_FOUND_BUT_INVALID:
            return INET_NOT_IMPLEMENTED;

        default:
            return INET_ERROR;
    }
}

int inet_resolve_ip_address(const uint8_t* ipv4_addr, uint8_t* hw_addr) { // TODO
    uint32_t entry_index;
    int err = find_arp_entry_by_ipv4(ipv4_addr, &entry_index);
        
    if (err == INET_SUCCESS) {
        memcpy(hw_addr, _arp_table[entry_index].hw_addr, MAC_ADDR_LENGTH);
    }
    
    return err;
}
int inet_resolve_hw_address(const uint8_t* hw_addr, uint8_t* ipv4_addr) { // TODO
    uint32_t entry_index;
    int err = find_arp_entry_by_mac(hw_addr, &entry_index);
        
    if (err == INET_SUCCESS) {
        memcpy(ipv4_addr, _arp_table[entry_index].ip_addr, IPV4_ADDR_LENGTH);
    }
    
    return err;
}


/**********************************************************
 * Handler
 */

int inet_arp_handler(const uint8_t* data, uint16_t nbytes) {

    // Check length of message
    if (nbytes != ARP_MESSAGE_BYTES) {
        trace("Invalid ARP message length. Read %d. Expected %d\n",
            nbytes, ARP_MESSAGE_BYTES);
        return INET_ARP_INVALID_MSG_LEN;
    }

    int err;
    arp_packet_t* arp = (arp_packet_t*)data;

    // ---------- 0. Swap endian ----------
    swapEndian16(&arp->hardware_type);
    swapEndian16(&arp->protocol_type);
    swapEndian16(&arp->operation);
            
    if (verbose_p)
        trace("GOT ARP\n");

    // ---------- 2. Check packet ----------

    // Physical, RJ45 ethernet
    if (arp->hardware_type != ARP_HTYPE_ETHERNET) {return INET_ARP_BAD_HTYPE; }
    if (arp->hardware_len != MAC_ADDR_LENGTH) { return INET_ARP_INVALID_MAC_LEN; }

    // IPV4 addresses
    if (arp->protocol_type != FRAME_IPV4) { return INET_ARP_BAD_HTYPE; }
    if (arp->protocol_len != IPV4_ADDR_LENGTH) { return INET_ARP_INVALID_IP_LEN; }

    // print_bytes("ARP PACKET:", (void*)data, ARP_MESSAGE_BYTES);
    // print_as_string("ARP PACKET:", (void*)data, ARP_MESSAGE_BYTES);

    // ---------- 3. Logic ----------

    // "?Am I the target protocol address?" -rfc
    if (memcmp(arp->dest_ipv4_addr, inet_get_ipv4_addr(), IPV4_ADDR_LENGTH) != 0) {
        return INET_ARP_NOT_FOR_US;
    }

    // ---------- 4. Add to table ----------
    // "add <protocol type, sender protocol address, sender hardware address> to the translation table" -rfc
    inet_add_arp_entry(arp->src_ipv4_addr, arp->src_hw_addr); 

    // ?Is the opcode ares_op$REQUEST? -rfc
    switch (arp->operation) {
        case ARP_REQUEST:
            if (verbose_p)
                trace("Received ARP request from %d.%d.%d.%d\n",
                    arp->src_ipv4_addr[0],
                    arp->src_ipv4_addr[1],
                    arp->src_ipv4_addr[2],
                    arp->src_ipv4_addr[3]);
            return inet_send_arp(arp->src_hw_addr, arp->src_ipv4_addr, ARP_REPLY);
            
        case ARP_REPLY: // Do nothing
            return INET_SUCCESS;

        default:
            return INET_ARP_INVALID_OP;
    }
}

// https://www.rfc-editor.org/rfc/rfc826.html
int inet_send_arp(const uint8_t* their_hw_addr, const uint8_t* their_ipv4_addr, uint8_t operation) {

    if (operation != ARP_REPLY && operation != ARP_REQUEST) {
        trace("Invalid ARP operation (%x)\n", operation);
        return INET_ARP_INVALID_OP;
    }

    // ---------- 1. Make and populate packet ----------
    arp_packet_t arp;
    arp.hardware_type = ARP_HTYPE_ETHERNET;
    arp.protocol_type = FRAME_IPV4;
    arp.hardware_len = MAC_ADDR_LENGTH;
    arp.protocol_len = IPV4_ADDR_LENGTH;
    arp.operation = operation;

    memcpy(arp.src_hw_addr, inet_get_hw_addr(), MAC_ADDR_LENGTH);
    memcpy(arp.src_ipv4_addr, inet_get_ipv4_addr(), IPV4_ADDR_LENGTH);
    memcpy(arp.dest_hw_addr, their_hw_addr, MAC_ADDR_LENGTH);
    memcpy(arp.dest_ipv4_addr, their_ipv4_addr, IPV4_ADDR_LENGTH);

    // ---------- 2. Swap endian ----------
    swapEndian16(&arp.hardware_type);
    swapEndian16(&arp.protocol_type);
    swapEndian16(&arp.operation);

    return INET_SUCCESS;

    return inet_send_frame(their_hw_addr, FRAME_ARP, &arp, ARP_MESSAGE_BYTES);
}


/**********************************************************
 * Helpers
 */

void print_arp_entry(const uint8_t* ip_addr, const uint8_t* hw_addr, int newline) {
    printk("{%d.%d.%d.%d} --> {%X:%X:%X:%X:%X:%X}",
            ip_addr[0],
            ip_addr[1],
            ip_addr[2],
            ip_addr[3],
            hw_addr[0],
            hw_addr[1],
            hw_addr[2],
            hw_addr[3],
            hw_addr[4],
            hw_addr[5]
        );
    if (newline)
        printk("\n");
}

void trace_arp_table(const char* msg) {
    
    // This is cooked
    trace_nofn("%s\n", msg);
    for(int i = 0; i < ARP_TABLE_SIZE; i++) {
        trace_nofn("%d.\t", i);
        print_arp_entry(_arp_table[i].ip_addr, _arp_table[i].hw_addr, 0);

        if (_arp_table[i].valid)
            printk(" Valid\n");
        else
            printk(" Invalid\n");
    }
    trace_nofn("\n");
 }


//  // print [p, p+n) as a string: use for ascii filled files.
// static inline void print_as_string(const char *msg, uint8_t *p, int n) {

// }


