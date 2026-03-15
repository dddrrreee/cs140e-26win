from scapy.layers.l2 import Ether, ARP
from scapy.layers.inet import IP, UDP
from scapy.packet import Raw
from scapy.sendrecv import sendp
from time import sleep
import random

"""
Sends UDP to the Pi (192.168.0.3 / 76:67:67:67:67:67) on:
- port 40000: onboard LED handler (message content used for morse/blink)
- port 40001: parthiv LED handler (any message toggles)

It first sends an ARP packet so the Pi can learn our (SCRIPT_IP -> SCRIPT_MAC).
"""


PI_IP = "192.168.0.3"
PI_MAC = "76:67:67:67:67:67"
BROADCAST_MAC = "ff:ff:ff:ff:ff:ff"

SCRIPT_IP = "1.2.3.4"
SCRIPT_MAC = "00:11:22:33:44:55"
ONBOARD_LED_PORT = 40000
PARTHIV_LED_PORT = 40001

INTERFACE = "en13"

def arp_start():
    # Tell the Pi: "1.2.3.4 is at 00:11:22:33:44:55"
    arp_packet = Ether(dst=PI_MAC, src=SCRIPT_MAC) / ARP(
        op=1,              # who-has (request)
        hwsrc=SCRIPT_MAC,
        psrc=SCRIPT_IP,
        pdst=PI_IP,
    )
    sendp(arp_packet, iface=INTERFACE, verbose=False)


def send_udp(dport: int, payload: bytes, sport: int = 24087):
    frame = (
        Ether(dst=PI_MAC, src=SCRIPT_MAC)
        / IP(src=SCRIPT_IP, dst=PI_IP)
        / UDP(sport=sport, dport=dport)
        / Raw(load=payload)
    )
    sendp(frame, iface=INTERFACE, verbose=False)


if __name__ == "__main__":
    arp_start()
    sleep(0.5)

    while True:

        cmd = input("What to do (p/o/q)> ").strip()
        if not cmd:
            continue

        c = cmd[0]
        if c in ("q", "Q"):
            break
        elif c in ("p", "P"):
            send_udp(PARTHIV_LED_PORT, b"toggle")
            print("Sent to Parthiv LED (40001).")
        elif c in ("o", "O"):
            # If you want to type a custom message, replace b"HELLO" with msg.encode()
            msg = input("Onboard message: ").encode("utf-8", errors="replace")
            send_udp(ONBOARD_LED_PORT, msg)
            print("Sent to Onboard LED (40000).")
        else:
            print("Unknown command. Use p/o/q.")