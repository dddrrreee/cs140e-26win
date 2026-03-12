

from scapy.layers.l2 import Ether, ARP
from scapy.sendrecv import sendp
from time import sleep

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
    