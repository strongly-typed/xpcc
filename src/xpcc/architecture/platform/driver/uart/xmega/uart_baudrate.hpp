// coding: utf-8
/* Copyright (c) 2009, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef XPCC_ATXMEGA_UART_BAUDRATE_HPP
#define XPCC_ATXMEGA_UART_BAUDRATE_HPP

#include "../../../device.hpp"

namespace xpcc
{

namespace xmega
{

/**
 * This class provides several helper functions to calculate the values for
 * the fractional baudrate generator.
 *
 * All of these functions are `constexpr` functions which allow compile time
 * evaluation.
 * If you really need to change the baudrate at runtime, consider calculating
 * the register values at compile time, storing them in your program, then
 * switching them at runtime.
 * This will save you a *lot* of flash and execution time.
 *
 * @author	strongly-typed
 * @author	Niklas Hauser
 * @ingroup	xmega
 */
class UartBaudrate
{
public:
	/**
	 * Calculates the best BSCALE value.
	 *
	 * @param	baudrate
	 * 		Desired baud rate.
	 * @return	Unsigned BSCALE value.
	 */
	static constexpr uint8_t
	bScale(uint32_t baudrate)
	{
		return (bScaleSigned(baudrate) < 0) ? 16 + bScaleSigned(baudrate) : bScaleSigned(baudrate);
	}

	/**
	 * Caluculates the best BSEL value.
	 *
	 * @param	baudrate
	 * 		Desired baud rate.
	 */
	static constexpr uint16_t
	bSel(uint32_t baudrate)
	{
		return bSelFromBaudscale(baudrate, bScaleSigned(baudrate));
	}

private:
	/**
	 * Find the BSEL value depending on the BSCALE value and baudrate.
	 *
	 * @param	baudrate
	 * 		Desired baudrate
	 * @param	bscale
	 * 		The fractional divider.
	 */
	static constexpr uint16_t
	bSelFromBaudscale(uint32_t baudrate, int8_t bscale)
	{
		// The formulas are from table 23-1 on page 293 of the datasheet.
		return (bscale < 0) ?
				((static_cast<float>(F_CPU) /
						(16.f * static_cast<float>(baudrate))) - 1) * (1 << (-bscale)) :
				((static_cast<float>(F_CPU) /
						(16.f * (1 << bscale) * static_cast<float>(baudrate))) - 1);
	}

	/**
	 * Find a suitable BSCALE value for a given baudrate by trial-and-error.
	 *
	 * @param	baudrate
	 * 		Desired baudrate.
	 * @return	Signed BSCALE value.
	 */
	static constexpr int8_t
	bScaleSigned(uint32_t baudrate)
	{
		/*
		 * For 8N1 frames (1 start bit, 8 data bits, no parity, 1 stop bit)
		 * the minimum BSEL value is -6 and the maximum +6.
		 * (Datasheet, page 301, 23-9)
		 */
		return  (bSelFromBaudscale(baudrate, -6) < 4096) ? -6 : (
				(bSelFromBaudscale(baudrate, -5) < 4096) ? -5 : (
				(bSelFromBaudscale(baudrate, -4) < 4096) ? -4 : (
				(bSelFromBaudscale(baudrate, -3) < 4096) ? -3 : (
				(bSelFromBaudscale(baudrate, -2) < 4096) ? -2 : (
				(bSelFromBaudscale(baudrate, -1) < 4096) ? -1 : (
				(bSelFromBaudscale(baudrate,  0) < 4096) ?  0 : (
				(bSelFromBaudscale(baudrate, +1) < 4096) ? +1 : (
				(bSelFromBaudscale(baudrate, +2) < 4096) ? +2 : (
				(bSelFromBaudscale(baudrate, +3) < 4096) ? +3 : (
				(bSelFromBaudscale(baudrate, +4) < 4096) ? +4 : (
				(bSelFromBaudscale(baudrate, +5) < 4096) ? +5 : (
						+6	))))))))))));
	}
};

} // xmega namespace

} // xpcc namespace

#endif // XPCC_ATXMEGA_UART_BAUDRATE_HPP

