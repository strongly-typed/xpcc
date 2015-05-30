// coding: utf-8
/* Copyright (c) 2011, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#include <stdio.h>		// snprintf()
#include <stdlib.h>

#include <xpcc/utils/arithmetic_traits.hpp>
#include <xpcc/utils/template_metaprogramming.hpp>

#include "iostream.hpp"

void
xpcc::IOStream::writeFloat(const float& value)
{
#if defined(XPCC__CPU_AVR)
	// hard coded for -2.22507e-308
	char str[13 + 1]; // +1 for '\0'
	dtostre(value, str, 5, 0);
	while(*str) {
		this->device->write(*str++);
	}
#elif defined(XPCC__CPU_CORTEX_M4) || defined(XPCC__CPU_CORTEX_M3) || defined(XPCC__CPU_CORTEX_M0)
	float v;

	if (value < 0) {
		v = -value;
		this->device->write('-');
	}
	else {
		v = value;
	}

	int32_t ep = 0;
	if (v != 0)
	{
		while (v < 1.f) {
			v *= 10;
			ep -= 1;
		}

		while (v > 10) {
			v *= 0.1f;
			ep += 1;
		}
	}

	for (uint32_t i = 0; i < 6; ++i)
	{
		int8_t num = static_cast<int8_t>(v);
		this->device->write(num + '0');

		if (i == 0) {
			this->device->write('.');
		}

		// next digit
		v = (v - num) * 10;
	}

	this->device->write('e');
	if (ep < 0) {
		ep = -ep;
		this->device->write('-');
	}
	else {
		this->device->write('+');
	}
	if (ep < 10) {
		this->device->write('0');
	}

	this->writeInteger(ep);
#else
	// hard coded for -2.22507e-308
	char str[13 + 1]; // +1 for '\0'
	snprintf(str, sizeof(str), "%.5e", (double) value);
	this->device->write(str);
#endif
}

// ----------------------------------------------------------------------------
#if !defined(XPCC__CPU_AVR)
void
xpcc::IOStream::writeDouble(const double& value)
{
#if defined(XPCC__CPU_CORTEX_M4) || defined(XPCC__CPU_CORTEX_M3) || defined(XPCC__CPU_CORTEX_M0)
	// TODO do this better
	writeFloat(static_cast<float>(value));
#else
	// hard coded for 2.22507e-308
	char str[13 + 1]; // +1 for '\0'
	snprintf(str, sizeof(str), "%.5e", value);
	this->device->write(str);
#endif
}
#endif
