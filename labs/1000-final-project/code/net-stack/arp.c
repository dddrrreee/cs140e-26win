// Layer 2 (high-ish)

#include "arp.h"

#include "data-link.h" // For setting mac
#include "ipv4.h" // For setting ip

#include "../print-utilities.h"
#include "../endian.h"

static arp_table_entry_t _arp_table[ARP_TABLE_SIZE];

static int _verbose_p = 0;

arp_packet_t _arp_tx;

void arp_init(const verbose_t* verbosity) {
    if (verbosity->arp || verbosity->all) {
        _verbose_p = 1;
        trace("ARP verbosity enabled\n");
    }

    inet_clear_arp_table();
    trace("ARP table initalized (cleared)\n");
}

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
        if (memcmp(_arp_table[i].ip_addr, ipv4_addr, IPV4_ADDR_BYTES) == 0 ) {
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

        if (memcmp(_arp_table[i].hw_addr, hw_addr, MAC_ADDR_BYTES) == 0 ) {
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
            memcpy(_arp_table[idx].hw_addr, their_hw_addr, MAC_ADDR_BYTES);
            memcpy(_arp_table[idx].ip_addr, their_ipv4_addr, IPV4_ADDR_BYTES);
            _arp_table[idx].valid = 1;

            trace("ARP table updated: {%d.%d.%d.%d} -> {%X:%X:%X:%X:%X:%X}\n",
                their_ipv4_addr[0], their_ipv4_addr[1],
                their_ipv4_addr[2], their_ipv4_addr[3],
                their_hw_addr[0], their_hw_addr[1], their_hw_addr[2],
                their_hw_addr[3], their_hw_addr[4], their_hw_addr[5]);
                
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
        
    if (err == INET_SUCCESS)
        memcpy(hw_addr, _arp_table[entry_index].hw_addr, MAC_ADDR_BYTES);
    else
        memcpy(hw_addr, MAC_BROADCAST, MAC_ADDR_BYTES);
        
    
    return err;
}

int inet_resolve_hw_address(const uint8_t* hw_addr, uint8_t* ipv4_addr) { // TODO
    uint32_t entry_index;
    int err = find_arp_entry_by_mac(hw_addr, &entry_index);
        
    if (err == INET_SUCCESS)
        memcpy(ipv4_addr, _arp_table[entry_index].ip_addr, IPV4_ADDR_BYTES);
    else
        memcpy(ipv4_addr, IPV4_BROADCAST, IPV4_ADDR_BYTES);
    
    return err;
}


/**********************************************************
 * Handler
 */

int inet_arp_handler(const uint8_t* data, uint16_t nbytes) {

    if (_verbose_p)
        trace("Received ARP packet of %d bytes\n", nbytes);

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
    



    // ---------- 2. Check packet ----------

    // Physical, RJ45 ethernet
    if (arp->hardware_type != ARP_HTYPE_ETHERNET) { 
        trace("Invalid ARP hardware type %x\n", arp->hardware_type);
        return INET_ARP_BAD_HTYPE; 
    }
    if (arp->hardware_len != MAC_ADDR_BYTES) { 
        trace("Invalid ARP hardware length %d\n", arp->hardware_len);
        return INET_ARP_INVALID_MAC_LEN; 
    }
    
    
    // IPV4 addresses
    if (arp->protocol_type != FRAME_IPV4) { 
        trace("Invalid ARP protocol type %x\n", arp->protocol_type);
        return INET_ARP_BAD_HTYPE; 
    }
    if (arp->protocol_len != IPV4_ADDR_BYTES) { 
        trace("Invalid ARP protocol length %d\n", arp->protocol_len);
        return INET_ARP_INVALID_IP_LEN; 
    }
    

    // ---------- 3. Logic ----------

    // "?Am I the target protocol address?" -rfc
    if (memcmp(arp->dest_ipv4_addr, inet_get_ipv4_addr(), IPV4_ADDR_BYTES) != 0) {
        if (_verbose_p)
            trace("ARP packet not for us, but for {%d.%d.%d.%d}\n",
                arp->dest_ipv4_addr[0], arp->dest_ipv4_addr[1], arp->dest_ipv4_addr[2], arp->dest_ipv4_addr[3]);
        return INET_ARP_NOT_FOR_US;
    }

    // ---------- 4. Add to table ----------
    // "add <protocol type, sender protocol address, sender hardware address> to the translation table" -rfc
    inet_add_arp_entry(arp->src_ipv4_addr, arp->src_hw_addr);  // Adds no matter what (not sure if I should trace or not)

    // ?Is the opcode ares_op$REQUEST? -rfc
    switch (arp->operation) {
        case ARP_REQUEST:
            return inet_send_arp(arp->src_hw_addr, arp->src_ipv4_addr, ARP_REPLY);
            
        case ARP_REPLY: // Do nothing. not passing through
            if (_verbose_p)
                trace("ARP from {%d.%d.%d.%d} is reply. Doing nothing now\n",
                    arp->src_ipv4_addr[0], arp->src_ipv4_addr[1], arp->src_ipv4_addr[2], arp->src_ipv4_addr[3]);
            return INET_ARP_RECEIVED;
    }
    return INET_ARP_INVALID_OP;
}

// https://www.rfc-editor.org/rfc/rfc826.html
int inet_send_arp(const uint8_t* their_hw_addr, const uint8_t* their_ipv4_addr, uint8_t operation) {

    if (operation != ARP_REPLY && operation != ARP_REQUEST) {
        if (_verbose_p)
            trace("Invalid ARP operation (%x)\n", operation);
        return INET_ARP_INVALID_OP;
    }

    // ---------- 1. Make and populate packet ----------
    memset(&_arp_tx, 0, sizeof(_arp_tx));
    _arp_tx.hardware_type = ARP_HTYPE_ETHERNET;
    _arp_tx.protocol_type = FRAME_IPV4;
    _arp_tx.hardware_len = MAC_ADDR_BYTES;
    _arp_tx.protocol_len = IPV4_ADDR_BYTES;
    _arp_tx.operation = operation;

    memcpy(_arp_tx.src_hw_addr, inet_get_hw_addr(), MAC_ADDR_BYTES);
    memcpy(_arp_tx.src_ipv4_addr, inet_get_ipv4_addr(), IPV4_ADDR_BYTES);
    memcpy(_arp_tx.dest_hw_addr, their_hw_addr, MAC_ADDR_BYTES);
    memcpy(_arp_tx.dest_ipv4_addr, their_ipv4_addr, IPV4_ADDR_BYTES);

    // ---------- 2. Swap endian ----------
    swapEndian16(&_arp_tx.hardware_type);
    swapEndian16(&_arp_tx.protocol_type);
    swapEndian16(&_arp_tx.operation);

    trace("Sending ARP reply to {%d.%d.%d.%d}\n",
        their_ipv4_addr[0], their_ipv4_addr[1], their_ipv4_addr[2], their_ipv4_addr[3]);

    return inet_send_frame(their_hw_addr, FRAME_ARP, &_arp_tx, ARP_MESSAGE_BYTES);
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


