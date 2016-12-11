#!/usr/bin/env python2

import socket
from struct import *
import datetime
import pcap
import sys

def parse_packet(packet) :

	#parse ethernet header
	eth_length = 14

	eth_header = packet[:eth_length]
	eth = unpack('!6s6sH' , eth_header)
	eth_protocol = socket.ntohs(eth[2])
	print('Destination MAC : ' + eth_addr(packet[0:6]) + ' Source MAC : ' + eth_addr(packet[6:12]) + ' Protocol : ' + str(eth_protocol))

def recv_pkts(timestamp, packet):
	if len(packet) == 64:
		if packet[0:4] == '\x8eRCA':
			print(timestamp),
			print(': '),
			print(' '.join('{:02x}'.format(ord(x)) for x in packet[0:32]))

cap = pcap.pcap(name='en4' , snaplen=256, promisc=True, immediate=True)

packet_limit = -1
try:
	cap.loop(cnt=packet_limit, callback=recv_pkts) # capture packets
except KeyboardInterrupt:
	pass
