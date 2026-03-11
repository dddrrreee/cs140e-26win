

from scapy.all import Ether, sendp, ICMP, IP
from time import sleep

while(1):
    icmp_frame = Ether(dst="ff:ff:ff:ff:ff:ff", src="00:11:22:33:44:55")/IP(dst="192.168.1.1", src="1.1.1.1")/ICMP(type=8, code=0)/b"Hello, W5500!"
    sendp(icmp_frame, iface="en13")
    sleep(1)