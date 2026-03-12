from scapy.layers.l2 import Ether, ARP
from scapy.layers.inet import IP, UDP
from scapy.packet import Raw
from scapy.sendrecv import sendp
from time import sleep

PI_IP = "192.168.0.3"
PI_MAC = "76:67:67:67:67:67"
BROADCAST_MAC = "ff:ff:ff:ff:ff:ff"

SCRIPT_IP = "1.2.3.4"
SCRIPT_MAC = "00:11:22:33:44:55"

frame = Ether(dst=BROADCAST_MAC) / IP(src=SCRIPT_IP, dst=PI_IP) / UDP(sport=42069, dport=42069) / Raw(load=b"Hello DHCP")

if __name__ == "__main__":

    # Send ARP to have PI link 
    arp_initial_packet = Ether(dst=PI_MAC) / ARP(pdst=PI_IP, psrc=SCRIPT_IP)
    sendp(frame, iface="en13")

    sleep(1)

    sendp(frame, iface="en13")