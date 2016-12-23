#!/usr/bin/env python2

from zlib import crc32

def reverse_mask(x):
	x = ((x & 0x55555555) <<  1) | ((x & 0xAAAAAAAA) >>  1)
	x = ((x & 0x33333333) <<  2) | ((x & 0xCCCCCCCC) >>  2)
	x = ((x & 0x0F0F0F0F) <<  4) | ((x & 0xF0F0F0F0) >>  4)
	x = ((x & 0x00FF00FF) <<  8) | ((x & 0xFF00FF00) >>  8)
	x = ((x & 0x0000FFFF) << 16) | ((x & 0xFFFF0000) >> 16)
	return x

def calc_hash(data):
	return reverse_mask(crc32(data))

def calc_hash_table(data):
	return (calc_hash(data) >> 26) & 0x3f

if __name__ == '__main__':
	print('STM32 Ethernet Hash Table Calculation Method')

	test_vector = []

	# Test vectors from STM32F4 Reference Manual (RM0090, page 1188)
	test_vector.append({
		'data' : b'\x1f\x52\x41\x9c\xb6\xaf',
		'expected' : ord(b'\x2c')
	})
	test_vector.append({
		'data' : b'\xa0\x0a\x98\x00\x00\x45',
		'expected' : ord(b'\x07')
		})

	# Test vector from own research: verified that the filter matches
	test_vector.append({
		'data' : b'\x53\x43\x41\x00\x00\x13',
		'expected' : ord(b'\x29')
		})

	test_result = True
	for item in test_vector:
		print('Checking %s:' % ''.join(('%02x' % ord(x)) for x in item['data'])),
		c = calc_hash_table(item['data'])
		e = item['expected']
		r = (c == e)

		HTH = 0
		HTL = 0

		if (c & 0x20):
			HTH = (1 << (c & 0x1f))
		else:
			HTL = (1 << (c & 0x1f))

		if not r:
			test_result = False
		print('Calced: %02x, Expected %02x, Result: %s. HTH = 0x%08x, HTL = 0x%08x' % (c, e, r, HTH, HTL))
	print('Overall Test Result: %s' % test_result)
