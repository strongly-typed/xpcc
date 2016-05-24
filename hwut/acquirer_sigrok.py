import subprocess
import logging
from os import close, unlink
from time import sleep
from tempfile import mkstemp

class SigrokAcquirer:
	def __init__(self, config):
		self.config = config

	def start(self, aconfig):
		fd, self.sroutfile = mkstemp()
		close(fd)

		cmd = ['/usr/local/bin/sigrok-cli', 
				'--driver', self.config['driver'], 
				'--config', ','.join(['%s=%s' % (key,value) for (key, value) in aconfig['config'].iteritems()]), 
				'--samples', aconfig['samples'],
				'--channels', ','.join(['%s=%s' % (key,value) for (key, value) in self.config['channels'].iteritems()]),
				'--triggers', ','.join(['%s=%s' % (key,value) for (key, value) in aconfig['triggers'].iteritems()]),
				'--wait-trigger',
				'--output-file', self.sroutfile,
				]
		self.p = subprocess.Popen(cmd)

		# Wait for sigrok to settle and arm trigger
		sleep(0.5)


	def waitForCompletion(self, timeout=10):
		# Todo: Timeout is samples / samplerate + 3 seconds
		while timeout != 0:
			if self.p.poll() is not None:
				break
			logging.info('Sigrok: Timeout = %d' % timeout)
			timeout -= 1
			sleep(1)

		if timeout == 0:
			self.p.kill()
			logging.warning('Sigrok: Termination by timeout')
			os.unlink(self.sroutfile)
			return False
		else:
			logging.info('Sigrok: Normal termination')
			return True

	def getOutputFilename(self):
		return self.sroutfile

	def close(self):
		unlink(self.sroutfile)
