

from scapy.all import Ether, sendp, ICMP, IP, ARP
from time import sleep

frame = Ether(dst="ff:ff:ff:ff:ff:ff", src="00:11:22:33:44:55")/IP(dst="192.168.0.3", src="1.1.1.1")/ICMP(type=8, code=0)/b"Hello, W5500!"

# frame = Ether(dst="ff:ff:ff:ff:ff:ff") / ARP(pdst="192.168.0.3")
while(1):
    sendp(frame, iface="en13")
    sleep(1)
    