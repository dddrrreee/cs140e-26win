from scapy.layers.l2 import Ether, ARP
from scapy.layers.inet import IP, ICMP
from scapy.sendrecv import sniff, sendp
from scapy.packet import Packet

PI_IP = "192.168.0.3"
PI_MAC = "76:67:67:67:67:67"

SCRIPT_IP = "1.2.3.4"
SCRIPT_MAC = "00:11:22:33:44:55"


def handle_frame(pkt: Packet):
    if pkt.haslayer(Ether):
        eth = pkt[Ether]
        if eth.dst == SCRIPT_MAC:
            print(f"RPi sent frame to {SCRIPT_MAC}")
            # Respond with ICMP reply
            ip = pkt.getlayer(IP)
            if ip:
                # `src` is the PI, `dst` is SCRIPT
                icmp_reply = Ether(dst=eth.src, src=eth.dst)/IP(dst=ip.src, src=ip.dst)/ICMP(type=0)/b'ping reply'
                sendp(icmp_reply, iface="en13")

if __name__ == "__main__":

    # Send ARP to have PI link 
    arp_initial_packet = Ether(dst=PI_MAC, src=SCRIPT_MAC) / ARP(pdst=PI_IP, psrc=SCRIPT_IP, hwsrc=SCRIPT_MAC)
    sendp(arp_initial_packet, iface="en13")
        

    # Try to read packet
    sniff(iface="en13", prn=handle_frame)