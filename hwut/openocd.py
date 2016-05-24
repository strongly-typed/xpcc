import telnetlib

class Target(object):
	def __init__(self, address='localhost', port=4444):
		self.address = address
		self.tn = telnetlib.Telnet(address, port)
		self.TIMEOUT = 1.0

	def init(self):
		pass

	def halt(self):
		self.tn.write('halt\n')
		(result, mobj, str) = self.tn.expect(["(.*)\n>"], self.TIMEOUT)

	def resume(self):
		self.tn.write('resume\n')
		(result, mobj, str) = self.tn.expect(["(.*)\n>"], self.TIMEOUT)

	def reset(self):
		self.tn.write('reset run\n')
		(result, mobj, str) = self.tn.expect(["(.*)\n>"], self.TIMEOUT)

	def resetStopOnReset(self):
		self.tn.write('reset halt\n')
		(result, mobj, str) = self.tn.expect(["(.*)\n>"], self.TIMEOUT)

if __name__ == "__main__":
	pass
