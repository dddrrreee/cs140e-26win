from scapy.layers.l2 import Ether, ARP
from scapy.layers.inet import IP, UDP
from scapy.packet import Raw
from scapy.sendrecv import sendp
from time import sleep

PI_IP = "192.168.0.3"
PI_MAC = "76:67:67:67:67:67"
PI_PORT = 8080
BROADCAST_MAC = "ff:ff:ff:ff:ff:ff"

SCRIPT_IP = "1.2.3.4"
SCRIPT_MAC = "00:11:22:33:44:55"
SCRIPT_PORT = 24087
frame = Ether(dst=BROADCAST_MAC) / IP(src=SCRIPT_IP, dst=PI_IP) / UDP(sport=SCRIPT_PORT, dport=PI_PORT) / Raw(load=b"Hello UDP")

if __name__ == "__main__":

    # Send ARP to have PI link 
    arp_initial_packet = Ether(dst=PI_MAC) / ARP(pdst=PI_IP, psrc=SCRIPT_IP)
    sendp(arp_initial_packet, iface="en13")

    sleep(1)

    sendp(frame, iface="en13")