#!/usr/bin/env python2
import zlib
import pprint

hashes = {}

dst = bytearray('\xe8RCA\x00\x00')
for d in range(256):
	for p in range(256):
		dst[4] = d
		dst[5] = p
		h = zlib.crc32(str(dst)) & 0x3f
		if hashes.has_key(h):
			hashes[h].append({'d':d, 'p':p})
		else:
			hashes[h] = [{'d':d, 'p':p}]
		# print('d=%02x p=%02x h=%2d' % (d, p, h))

# pprint.pprint(hashes)

for h, d in hashes.iteritems():
	print('%2d: %2d' % (h, len(d)))
