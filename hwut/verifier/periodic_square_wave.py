# Verifies a periodic square wave


class PeriodicSquareWave:
	def __init__(self, low, high, 
			period_tolerance = 0.01, 
			low_tolerance = 0.01,
			high_tolerance = 0.01):
		self.low = low
		self.high = high
		self.period = low + high
		self.period_tolerance = period_tolerance
		self.low_tolerance = low_tolerance
		self.high_tolerance = high_tolerance
		self.report = []

	def _min_max_tolerance(self, value, tolerance):
		return ((value * (1.0 - tolerance)), (value * (1.0 + tolerance)))

	def _trace_to_values(self, trace):
		values = []
		for line in trace:
			line = line.rstrip()
			value, unit = line.split(' ')

			units = { 's': 1, 'ms': 1E-3, 'us': 1E-6, 'ns': 1E-9, 'ps': 1E-12 }

			value = float(value) * units[unit]
			values.append(value)

		return values

	def _in_range(self, name, value, value_min, value_max):
		ok = False
		if ( ( value >= value_min) and ( value <= value_max) ):
			res = 'OK'
			ok = True
		else:
			res = 'NOK'
		self.report.append('%-3s %10s: %f <= %f <= %f' % (res, name, value_min, value, value_max))
		return ok


	def verify(self, trace):
		period_min, period_max = self._min_max_tolerance(self.period, self.period_tolerance)
		low_min,    low_max    = self._min_max_tolerance(self.low,    self.low_tolerance   )
		high_min,   high_max   = self._min_max_tolerance(self.high,   self.high_tolerance  )

		values = self._trace_to_values(trace)

		everythingOK = True
		for r in range(0, len(values) - 1, 2):
			low = values[r]
			high = values[r + 1]
			period = low + high

			everythingOK &= self._in_range('period', period, period_min, period_max)
			everythingOK &= self._in_range('low',    low,    low_min,    low_max)
			everythingOK &= self._in_range('high',   high,   high_min,   high_max)

		return everythingOK

	def getReport(self):
		return self.report

if __name__ == "__main__":
    # values = [0.2, 0.4, 0.3, 0.4]
    trace = "200.550 ms\r\n200.530 ms\r\n200.530 ms\r\n200.530 ms"
    psw = PeriodicSquareWave(0.2, 0.4)
    psw.verify(trace)
