

from scapy.all import Ether, sendp, ICMP, IP, ARP
from time import sleep

# icmp_frame = Ether(dst="ff:ff:ff:ff:ff:ff", src="00:11:22:33:44:55")/IP(dst="192.168.1.1", src="1.1.1.1")/ICMP(type=8, code=0)/b"Hello, W5500!"

# frame = Ether(dst="ff:ff:ff:ff:ff:ff") / ARP(pdst="192.168.0.3", src="1.2.3.4")

frames = []
for i in range(4):
    frames.append(
        Ether(dst="76:67:67:67:67:67", src=f"00:11:22:33:44:{i}") / ARP(pdst="192.168.0.3", psrc=f"1.2.3.{i}", hwsrc=f"00:11:22:33:44:{i}")
    )

while(1):

    for f in frames:
        print(f"Sending {f}")
        sendp(f, iface="en13")
        sleep(.5)
    
    exit()
    