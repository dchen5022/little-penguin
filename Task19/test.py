#!/usr/bin/env python3

from scapy.all import IP, TCP, send, Raw
import sys

PAYLOAD_STRING = b"682c83e55b77"
DEST_IP = "127.0.0.1"
DEST_PORT = 12345

def send_packet_with_string():
    packet = IP(dst=DEST_IP)/TCP(dport=DEST_PORT, flags="S")/Raw(load=PAYLOAD_STRING)
    send(packet, verbose=0)

if __name__ == "__main__":
    send_packet_with_string()
