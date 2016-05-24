import subprocess
import logging
from os import close, unlink
from time import sleep
from tempfile import mkstemp

class SigrokDecoder:
	def __init__(self):
		pass

	def decode(self, config, sroutfile):
		self.fd, self.srdecodefile = mkstemp()

		cmd = [
			'/usr/local/bin/sigrok-cli',
			'--input-file', sroutfile,
			'--protocol-decoders', (config.keys()[0] + ':' + ':'.join(['%s=%s' % (key,value) for (key, value) in config[config.keys()[0]].iteritems()]) if isinstance(config, dict) else config)
			]
		p = subprocess.Popen(cmd, stdout=self.fd)
		p.wait()

		close(self.fd)


	def getOutputFilename(self):
		return self.srdecodefile

	def close(self):
		unlink(self.srdecodefile)
