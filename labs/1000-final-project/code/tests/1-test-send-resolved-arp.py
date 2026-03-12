from scapy.layers.l2 import Ether, ARP
from scapy.layers.inet import IP, ICMP
from scapy.sendrecv import sniff, sendp
from scapy.packet import Packet
from time import sleep

PI_IP = "192.168.0.3"
PI_MAC = "76:67:67:67:67:67"
BROADCAST_MAC = "ff:ff:ff:ff:ff:ff"

SCRIPT_IP = "1.2.3.4"
SCRIPT_MAC = "00:11:22:33:44:55"

# ping_frame = Ether(dst=BROADCAST_MAC, src=SCRIPT_MAC) / ARP(pdst="192.168.0.3", psrc=f"1.2.3.{i}", hwsrc=f"00:11:22:33:44:{i}")
ping_frame = Ether(dst=BROADCAST_MAC)/IP(dst=PI_IP, src=SCRIPT_IP)/ICMP(type=8, code=0)/b"Give me ping again!"

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
    arp_initial_packet = Ether(dst=PI_MAC) / ARP(pdst=PI_IP, psrc=SCRIPT_IP)
    sendp(arp_initial_packet, iface="en13")

    sleep(1)

    sendp(ping_frame, iface="en13")

    # Try to read packet
    sniff(iface="en13", prn=handle_frame)